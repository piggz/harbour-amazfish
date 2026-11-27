#include "garminmlr.h"
#include <QtMath>
#include <QObject>
#include <QString>

#include <QtCore/QMetaObject>
#include "QEventLoop"

void MlrCommunicator::onMessageDecoded(const QByteArray& data) {
    emit gfdiDecoded(data);
}

MlrCommunicator::MlrCommunicator(quint8 handle,
                                 int maxPacketSize,
                                 QSharedPointer<QBLECharacteristic> sendChar,
                                 QObject* parent)
    : QObject(parent)
    , mRetransmissionTimer(new QTimer(this))
    , mAckTimer(new QTimer(this))
{
    qDebug() << "Garmin: Creating MLR communicator for handle" <<handle;
    mAckTimer->setSingleShot(true);
    mRetransmissionTimer->setSingleShot(true);

    mSentFragments.resize(MAX_SEQ_NUM + 1);
    mHandle = handle;
    mMaxPacketSize = maxPacketSize;
    mSendChar=sendChar;

    connect (mRetransmissionTimer,&QTimer::timeout,this,&MlrCommunicator::onRetransmissionTimeout);
    connect (mAckTimer,&QTimer::timeout,this,&MlrCommunicator::onAckTimeout);
    connect(&mCobsCodec,&CobsCoDec::messageDecoded, this, &MlrCommunicator::onMessageDecoded);
}

MlrCommunicator::~MlrCommunicator() {
    qDebug() << Q_FUNC_INFO <<"Garmin Mlr Communicator terminated";
}


void MlrCommunicator::setMaxPacketSize(int maxPacketSize) {
    mLocker.lock();
    mMaxPacketSize = maxPacketSize;
    mLocker.unlock();
    qDebug()<< "Garmin: MLR max packet size set to " << maxPacketSize;
}

void MlrCommunicator::sendMessage(const QString& taskName, const QByteArray& message) {
    if (message.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Empty Message  in MLR Commnicator";
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Queuing MLR message for " << taskName <<  "(" << message.size() <<" bytes)";

    const int maxDataSize = mMaxPacketSize - 2;
    if (maxDataSize <= 0) {
        qDebug() << Q_FUNC_INFO << "Garmin: Packet size leaves no room for MLR data" << mMaxPacketSize;
        return;
    }
    mLocker.lock();
    int remaining = message.size();
    int i = 0;
    if (remaining > maxDataSize) {
        int position = 0;
        while (remaining > 0) {
            const int chunk = qMin(remaining, maxDataSize);
            Fragment f;
            f.taskName = taskName;
            f.num = i++;
            f.data = message.mid(position, chunk);
            f.reqNum=0;
            mFragmentQueue.append(f);

            position += chunk;
            remaining -= chunk;
        }
    }
    else {
        Fragment f;
        f.taskName=taskName;
        f.num=0;
        f.data=message;
        f.reqNum=0;
        mFragmentQueue.append(f);
    }
    runProtocolOnce();
    mLocker.unlock();
}

void MlrCommunicator::onPacketReceived(const QByteArray& packet) {
    qDebug() << Q_FUNC_INFO;
    if (packet.size() < 2) {
        qDebug() << Q_FUNC_INFO << "Garmin: MLR packet too short.";
        return;
    }

    const quint16 byte0 = quint16(packet[0]) & 0xFF;
    const quint16 byte1 = quint16(packet[1]) & 0xFF;

    if ((byte0 & MLR_FLAG_MASK) == 0) {
        qDebug() << Q_FUNC_INFO << "Garmin:Received non-MLR packet";
        return;
    }

    const quint16 packetHandle = (byte0 & HANDLE_MASK) >> HANDLE_SHIFT;
    const quint16 reqNum = ((byte0 & REQ_NUM_MASK) << 2) | ((byte1 >> 6) & 0x03);
    const quint16 seqNum = byte1 & SEQ_NUM_MASK;


    if (packetHandle != (mHandle & 0x07)) {
        qDebug() << Q_FUNC_INFO << "Garmin:MLR packet for wrong handle: expected " << (mHandle & 0x07) << ", got " << packetHandle;
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: MLR packet received: reqNum=" << reqNum <<", seqNum=" << seqNum << ", dataLen=" << packet.size() - 2;

    // Process ACK if reqNum changed

    QByteArray data;
    mLocker.lock();
    data = processPacket(reqNum,seqNum,packet);
    mLocker.unlock();

    if (!data.isEmpty()) {
        mCobsCodec.feed(data);
    }
    mLocker.lock();
    if (!data.isEmpty())
        scheduleAck();
    runProtocolOnce();
    mLocker.unlock();
}

/**
 * Updates the ACK and sequence state for a received packet. Returns the payload when the
 * packet is in sequence, or null.
 */
QByteArray MlrCommunicator::processPacket(int reqNum, int seqNum, const QByteArray&  packet) {
    if (reqNum != mLastRcvAck) {
         processAck(reqNum);
    }
    if (packet.size() <=2)
        return QByteArray();
    if (seqNum != mNextRcvSeq) {
        // out of sequence
        qDebug() << Q_FUNC_INFO << "Gamin:  Out-of-sequence packet - expected " << mNextRcvSeq <<", got " << seqNum;
        sendAckPacket();
        return QByteArray();
    }
     mNextRcvSeq = (mNextRcvSeq + 1) % (MAX_SEQ_NUM + 1);
     return packet.mid(2);
}

void MlrCommunicator::onRetransmissionTimeout() {
    mLocker.lock();
    qDebug() << Q_FUNC_INFO << "Garmin: Retransmission timeout expired";
    // Backoff retransmission timeout and reduce the maximum unacked
    mRetransmissionTimeoutMs = qMin(mRetransmissionTimeoutMs * 2, MAX_RETRANSMISSION_TIMEOUT_MS);
    mMaxNumUnackedSend = qMax(1, mMaxNumUnackedSend / 2);


    for (int i = mLastRcvAck; i != mNextSendSeq; i = (i + 1) % (MAX_SEQ_NUM + 1)) {
        qDebug() << Q_FUNC_INFO << "Re-sending fragment " <<  i;
        if (!mSentFragments[i].has_value())
        {
            qWarning() << "MlrCommunicator: attempting to re-send null fragment at index" << i;
            continue;
        }
        const Fragment &fragment = *mSentFragments[i];
        const QByteArray packet = createPacket(fragment.reqNum, i, fragment.data);
        QString errorMsg;
        if (mSendChar) {
            mSendChar->writeValue(packet,&errorMsg);
            if (!errorMsg.isEmpty()) qDebug() << Q_FUNC_INFO << "Garmin: MlrMEssagesender send packet failed with error " <<errorMsg;
        }
     }
    startRetransmissionTimer();
    mLocker.unlock();
}


QByteArray MlrCommunicator::createPacket(int reqNum, int seqNum, const QByteArray& data) {
    QByteArray packet;
    packet.reserve(2 + data.size());

    const quint8 byte0 = quint8(MLR_FLAG_MASK |
                                ((mHandle & 0x07) << HANDLE_SHIFT) |
                                ((reqNum >> 2) & REQ_NUM_MASK));
    const quint8 byte1 = quint8(((reqNum & 0x03) << 6) | (seqNum & SEQ_NUM_MASK));

    packet.append(char(byte0));
    packet.append(char(byte1));
    packet.append(data);
    return packet;
}

void MlrCommunicator::processAck(int reqNum) {
    qDebug() << Q_FUNC_INFO << "Garmin: MLRCommunicator processing Ack";

    int numAcked = (reqNum - mLastRcvAck + MAX_SEQ_NUM + 1) %(MAX_SEQ_NUM +1);
    int numUnacked = (mNextSendSeq - mLastRcvAck + MAX_SEQ_NUM +1) % (MAX_SEQ_NUM +1);

    mRetransmissionTimer->stop();

    int ackBytes = 0;
    for (int i = mLastRcvAck; i != reqNum; i = (i + 1) % (MAX_SEQ_NUM + 1)) {
        if (!mSentFragments[i]) {
            mSentFragments[i].reset();
            qDebug() << Q_FUNC_INFO << "Garmin: Attermptin to expire null fragment";
        }
        else {
            ackBytes += mSentFragments[i]->data.size();
        }
        mSentFragments[i].reset();
    }
    mLastRcvAck = reqNum;

    //Recover from retransmission backoff
    mRetransmissionTimeoutMs = INITIAL_RETRANSMISSION_TIMEOUT_MS;
    mMaxNumUnackedSend = qMin(mMaxNumUnackedSend + numAcked, INITIAL_MAX_UNACKED_SEND);

    //Gadgetbridge now runs Acklistener, needs to be implemented

    if (mLastRcvAck != mNextSendSeq) {
        startRetransmissionTimer();
    }

}

void MlrCommunicator::onAckTimeout() {
    mLocker.lock();
    sendAckPacket();
    mLocker.unlock();
}

void MlrCommunicator::scheduleAck() {
    mAckTimer->stop();
    const int numRcvdUnacked = (mNextRcvSeq - mLastSendAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);
    if (numRcvdUnacked >= ACK_TRIGGER_THRESHOLD) {
        sendAckPacket();
    } else {
        mAckTimer->start(ACK_TIMEOUT_MS);
    }
}

void MlrCommunicator::sendAckPacket() {
    mAckTimer->stop();
    const QByteArray pkt = createPacket(mNextRcvSeq, 0, QByteArray());
    QString errorMsg;
    if (mSendChar) {
        mSendChar->writeValue(pkt,&errorMsg);
        if (!errorMsg.isEmpty()) qDebug() << Q_FUNC_INFO << "Garmin: Mlr send packet failed with error " <<errorMsg;
    }
    mLastSendAck = mNextRcvSeq;
}

void MlrCommunicator::runProtocolOnce() {
    qDebug() << Q_FUNC_INFO;
    Fragment frag;
    const int numSentUnacked = (mNextSendSeq - mLastRcvAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);
    if (numSentUnacked >= mMaxNumUnackedSend) {
        qDebug() << Q_FUNC_INFO << "Garmin: Cannot send more packets, " << numSentUnacked <<" unacked, max " << mMaxNumUnackedSend;
        return;
    }
    if (mFragmentQueue.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Empty Fragment Queue";
        return;
    }

    while (!mFragmentQueue.isEmpty()) {
        int numSentUnacked = (mNextSendSeq - mLastRcvAck + MAX_SEQ_NUM +1) % (MAX_SEQ_NUM +1);
        if (numSentUnacked >= mMaxNumUnackedSend) {
            qDebug() << Q_FUNC_INFO << "Garmin: Cannot send more packets.";
            return;
        }
        frag = mFragmentQueue.takeFirst();
        Fragment fragmentWithReqNum;
        fragmentWithReqNum.taskName=frag.taskName;
        fragmentWithReqNum.num=frag.num;
        fragmentWithReqNum.data=frag.data;
        fragmentWithReqNum.reqNum= mNextRcvSeq;
        const QByteArray pkt = createPacket(mNextRcvSeq,mNextSendSeq,frag.data);
        QString errorMsg;
        if (mSendChar) {
            qDebug() << Q_FUNC_INFO << "Garmin: Sending MlrFragment";
            mSendChar->writeValue(pkt,&errorMsg);
            if (!errorMsg.isEmpty()) qDebug() << Q_FUNC_INFO << "Garmin: MlrMessagesender send packet failed with error " <<errorMsg;
        }
        mSentFragments[mNextSendSeq] = fragmentWithReqNum;
        mNextSendSeq = (mNextSendSeq + 1) % (MAX_SEQ_NUM + 1);
        if (numSentUnacked == 0) {
            startRetransmissionTimer();
        }
        qDebug() << Q_FUNC_INFO << "Garmin: Sent MLR packet: seqNum=" << frag.num << ", dataLen=" << pkt.size() - 2;
    }
}


void MlrCommunicator::startRetransmissionTimer() {
    mRetransmissionTimer->start(mRetransmissionTimeoutMs);
}
