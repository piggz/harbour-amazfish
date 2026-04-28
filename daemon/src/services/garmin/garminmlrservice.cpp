#include "garminmlrservice.h"

#include <QApplication>

GarminMlrService::GarminMlrService( int handle,
                                    int maxPacketSize,
                                    void (*sendPacket)(QString taskName, QByteArray packet),
                                    void (*onDataReceived)(QByteArray data)
                                  ) : QObject()

{

    mHandle = handle;
    mMaxPacketSize = maxPacketSize;
    mSendPacket = sendPacket;
    mOnDataReceived = onDataReceived;
    mSentFragments.reserve(MAX_SEQ_NUM);
    mRetransmissionTimer = new QTimer(this);
    QObject::connect(mRetransmissionTimer,&QTimer::timeout,this,&GarminMlrService::onRetransmissionTimeout);
    mAckTimer = new QTimer(this);
    QObject::connect(mAckTimer,&QTimer::timeout,this,&GarminMlrService::sendAckPacket);

}

void GarminMlrService::setMaxPacketSize( int maxPacketSize)
{
    mMaxPacketSize = maxPacketSize;
}

void GarminMlrService::sendMessage(QString taskName, QByteArray message)
{
    if (message.length() == 0) {
        return;
    }

    qDebug() << "Queuing MLR message for '" << taskName << "' (" << message.length() <<" bytes)";

    int remainingBytes = message.length();
    int i = 0;
    if (remainingBytes > mMaxPacketSize - 2) {
        int position = 0;
        while (remainingBytes > 0) {
            QByteArray fragment = message.mid(position,std::min(remainingBytes,mMaxPacketSize-2));
            mFragmentQueue.append(Fragment(taskName,i++,fragment,0));
            position += fragment.length();
            remainingBytes -= fragment.length();
        }
    } else {
        mFragmentQueue.insert(mFragmentQueue.end(),Fragment(taskName,0,message,0));
    }

    runProtocol();
}

void GarminMlrService::onPacketReceived(QByteArray packet)
{
     if (packet.length() < 2) {
         qDebug() << "MLR packet too short: " << packet.length();
         return;
     }

     // MLR header
     static int byte0 = packet[0] & 0xFF;
     static int byte1 = packet[1] & 0xFF;

     if ((byte0 & MLR_FLAG_MASK) == 0) {
         qDebug() << "Received non-MLR packet";
         return;
     }

     static int packetHandle = (byte0 & HANDLE_MASK) >> HANDLE_SHIFT;
     static int reqNum = ((byte0 & REQ_NUM_MASK) << 2) | ((byte1 >> 6) & 0x03);
     static int seqNum = byte1 & SEQ_NUM_MASK;

     if (packetHandle != (mHandle & 0x07)) {
         qDebug() << "MLR packet for wrong handle: expected" << (mHandle & 0x07) << ", got " << packetHandle;
         return;
     }

     qDebug() <<
             "MLR packet received: reqNum=" <<reqNum << "seqNum=" << seqNum << ", datalen=" << packet.length() - 2;

     // Process ACK if request number changed
     if (reqNum != mLastRcvAck) {
         processAck(reqNum);
     }

     // Process data if any, and in sequence
     if (packet.length() > 2) {
         if (seqNum == mNextRcvSeq) {
             // In-sequence packet
             QByteArray data = packet.mid(2,packet.length());
             try {
                 mOnDataReceived(data);
             } catch (int e) {
                 qDebug() << "Receiver failed to handle MLR data" << e;
             }

             mNextRcvSeq = (mNextRcvSeq + 1) % (MAX_SEQ_NUM + 1);

             scheduleAckIfNeeded();
         } else {
             qDebug() << "Out-of-sequence packet - expected" << mNextRcvSeq << ", got " << seqNum;
             // Correct sequence will be retransmitted by sender
             // Regardless, re-send the expected ack since the sender shouldn't be sending these
             sendAckPacket();
         }
     }

     runProtocol();
 }

void GarminMlrService::processAck(const int reqNum)
{
    static int numAcked = (reqNum - mLastRcvAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);
    static int numUnacked = (mNextSendSeq - mLastRcvAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);

    qDebug() << "Processing ACK: reqNum=" << reqNum <<
                ", numAcked=" << numAcked <<
                ", numUnacked=" <<numUnacked <<
                ", will expire fragments [" << mLastRcvAck <<
               ", " << reqNum-1 << "]";

    // Stop retransmission timer
    mRetransmissionTimer->stop();

    // Remove acked messages from the array
    for (int i = mLastRcvAck; i != reqNum; i = (i + 1) % (MAX_SEQ_NUM + 1)) {
        if (i > mSentFragments.length()) {
            qDebug() <<"Attempting to expire null fragment at index" <<i;
        }
        mSentFragments.removeAt(i);
    }

    mLastRcvAck = reqNum;

    // Restart retransmission timer if there are still unacked packets
    if (mLastRcvAck != mNextSendSeq) {
        startRetransmissionTimer();
    }
}


void GarminMlrService::scheduleAckIfNeeded()
{
    mAckTimer->stop();

    static int numRcvdUnacked = (mNextRcvSeq - mLastSendAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);
    if (numRcvdUnacked >= ACK_TRIGGER_THRESHOLD) {
        sendAckPacket();
    } else {
        mAckTimer->start(ACK_TIMEOUT);
        qDebug() << "Started ack timer:" << ACK_TIMEOUT <<"ms";
    }
}



void GarminMlrService::sendAckPacket()
{
    mAckTimer->stop();
    // Send ACK-only packet (no data)
    QByteArray b;
    QByteArray packet = createPacket(mNextRcvSeq, 0, b);
    mSendPacket("ack reqNum=" + QString(mNextRcvSeq), packet);
    mLastSendAck = mNextRcvSeq;
    qDebug() <<"Sent ACK packet: reqNum=" << mNextRcvSeq;
}


void GarminMlrService::runProtocol() {
    // Check if we can send more packets
    int numSentUnacked = (mNextSendSeq - mLastRcvAck + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1);

    if (numSentUnacked >= mMaxNumUnackedSend) {
        qDebug() << "Cannot send more packets: " << numSentUnacked << " unacked, max " <<mMaxNumUnackedSend;
        return;
    }

    // Send next fragment if available
    if (!mFragmentQueue.empty())
    {
        Fragment fragment = mFragmentQueue.front();
        mFragmentQueue.pop_front();
        // Update the fragment with the current reqNum before storing it
        Fragment fragmentWithReqNum = Fragment(fragment.taskName, fragment.num, fragment.data, mNextRcvSeq);
        QByteArray packet = createPacket(mNextRcvSeq, mNextSendSeq, fragment.data);


        mSendPacket(fragment.taskName + " (" + fragment.num + ")", packet);
        mSentFragments[mNextSendSeq] = fragmentWithReqNum;

        mNextSendSeq = (mNextSendSeq + 1) % (MAX_SEQ_NUM + 1);

        // Start retransmission timer if this is the first unacked packet
        if (numSentUnacked == 0) {
            mRetransmissionTimer->start(mRetransmissionTimeout);
        }

        qDebug() << "Sent MLR packet: seqNum=" << (mNextSendSeq - 1 + MAX_SEQ_NUM + 1) % (MAX_SEQ_NUM + 1) << ", dataLen=" << fragment.data.length();

    }
}


QByteArray GarminMlrService::createPacket(const int reqNum, const int seqNum, const QByteArray data)
{
        QByteArray packet;
        packet.resize(2 + data.length());

        // First byte: MLR flag (1) + handle (3 bits) + reqNum high bits (4 bits)
        packet[0] =  (MLR_FLAG_MASK | ((mHandle & 0x07) << HANDLE_SHIFT) | ((reqNum >> 2) & REQ_NUM_MASK));

        // Second byte: reqNum low bits (2 bits) + seqNum (6 bits)
        packet[1] =  (((reqNum & 0x03) << 6) | (seqNum & SEQ_NUM_MASK));

        // Data
        packet.append(data);

        return packet;
    }

void GarminMlrService::startRetransmissionTimer() {
    mRetransmissionTimer->start(mRetransmissionTimeout);
    qDebug() << "Started retransmission timer: " <<  mRetransmissionTimeout << "ms";
}


void GarminMlrService::onRetransmissionTimeout() {
     qDebug() << "Retransmission timeout expired";

     // Backoff retransmission timeout and reduce the maximum unacked
     mRetransmissionTimeout = std::min(mRetransmissionTimeout * 2, MAX_RETRANSMISSION_TIMEOUT);
     mMaxNumUnackedSend = std::max(1, mMaxNumUnackedSend / 2);

     qDebug() <<"Retransmission: timeout=" << mRetransmissionTimeout << "ms, maxUnacked=" << mMaxNumUnackedSend << ", will re-send fragments [" << mRetransmissionTimeout << ", " << mNextSendSeq -1 << "]";

     for (int i = mLastRcvAck; i != mNextSendSeq; i = (i + 1) % (MAX_SEQ_NUM + 1)) {
         qDebug() << "Re-sending fragment " << i;
         static Fragment fragment = mSentFragments[i];
         if (fragment.data.isEmpty()) {
             qDebug() << "Attempting to re-send null fragment at index " << i;
             continue;
         }
         // Use the original reqNum that was stored when the fragment was first sent
         static QByteArray packet = createPacket(fragment.reqNum, i, fragment.data);
         mSendPacket("retransmission " + fragment.taskName + " (" + fragment.num + ")", packet);
     }

     startRetransmissionTimer();
 }

void GarminMlrService::close() {
    qDebug("Closing MLR communicator");

    mRetransmissionTimer->stop();

    mFragmentQueue.clear();
}

void GarminMlrService::onConnectionStateChange()
{
    mAckTimer->stop();
    /*
    if (newState != BluetoothGatt.STATE_CONNECTED) {
        timeoutHandler.removeCallbacksAndMessages(null);
    }
    */
}
