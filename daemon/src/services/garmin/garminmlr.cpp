#include "garminmlr.h"
#include <QtMath>
#include <QObject>
#include <QString>

#include <QtCore/QMetaObject>
#include "QEventLoop"


MlrMessageSender::MlrMessageSender(QSharedPointer<QBLECharacteristic> sendChar,
                           QObject* parent)
    : m_sendChar(sendChar)
{}


Result<void> MlrMessageSender::sendPacket(const QString& taskName, const QByteArray& packet) {
    // Rust logs hex dump; left to caller's logging.
    qDebug() << "Garmin: MlrMessagesender sending packet for task " << taskName;
    //auto r = awaitBleWrite(taskName, packet);
    QString errorMsg;
    if (m_sendChar) {
        m_sendChar->writeValue(packet,&errorMsg);
        if (!errorMsg.isEmpty()) qDebug() << Q_FUNC_INFO << "Garmin: MlrMEssagesender send packet failed with error " <<errorMsg;
    }
    return Result<void>::isOk();
}

// MlrMessageReceiver
// =============================================================================
// MLR Message Receiver implementation that processes decoded GFDI messages
//
// Data flow for incoming GFDI messages:
// 1. BLE receives packet with handle in first byte (e.g., 0x0B for GFDI)
// 2. MLR layer strips 2-byte MLR header, passes data here
// 3. This receiver COBS-decodes the data
// 4. Result is a GFDI message starting with packet size (2 bytes, little-endian)
// 5. GFDI message format: [size:2][msg_id:2][payload...][crc:2]
//
// Note: The handle byte (0x0B for GFDI) is NOT in the GFDI message itself.
// It's only used in BLE/MLR transport layers.

MlrMessageReceiver::MlrMessageReceiver(QSharedPointer<GfdiMessageCallback> syncCb,
                                 QPointer<AsyncGfdiMessageCallback> asyncCb,
                                 QObject* parent)
    : m_syncCb(std::move(syncCb))
    , m_asyncCb(asyncCb)
{}

Result<std::optional<QByteArray>> MlrMessageReceiver::awaitAsyncCallback(const QByteArray& message) {
    if (!m_asyncCb) return Result<std::optional<QByteArray>>::isOk(std::nullopt);

    const quint64 cookie = 1; // local cookie (receiver only)
    QEventLoop loop;
    Result<std::optional<QByteArray>> outcome = Result<std::optional<QByteArray>>::isOk(std::nullopt);

    QMetaObject::Connection c1 = QObject::connect(
        m_asyncCb, &AsyncGfdiMessageCallback::replyReady,
        &loop, [&](quint64 c, const QByteArray& reply){
            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::isOk(reply); loop.quit(); }
        });

    QMetaObject::Connection c2 = QObject::connect(
        m_asyncCb, &AsyncGfdiMessageCallback::noReply,
        &loop, [&](quint64 c){
            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::isOk(std::nullopt); loop.quit(); }
        });

    QMetaObject::Connection c3 = QObject::connect(
        m_asyncCb, &AsyncGfdiMessageCallback::failed,
        &loop, [&](quint64 c, const QString& err){
            if (c == cookie) { outcome = Result<std::optional<QByteArray>>::err(GarminError::invalidMessage(err)); loop.quit(); }
        });

    QMetaObject::invokeMethod(m_asyncCb, "onMessage", Qt::QueuedConnection,
                              Q_ARG(QByteArray, message),
                              Q_ARG(quint64, cookie));

    loop.exec();

    QObject::disconnect(c1);
    QObject::disconnect(c2);
    QObject::disconnect(c3);
    return outcome;
}

void MlrMessageReceiver::onDataReceived(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: Mlr Data received " << data.toHex();
    // The data from MLR is COBS encoded, so we need to decode it
    // Use persistent codec to support multi-packet messages
    m_codec.receiveBytes(data);

    auto decodedOpt = m_codec.retrieveMessage();
    if (!decodedOpt.has_value()) {
        return; // incomplete, wait for more
    }

    const QByteArray decoded = *decodedOpt;
    // The decoded message is the GFDI message directly - no handle byte here
    // The handle was already in the MLR header (stripped by MLR layer)
    if (decoded.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Warning: MLR decoded empty message";
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: Mlr decoded data " << decoded.toHex();

    emit gfdiDecoded(decoded);

    return;
}


void GfdiMessageCallback::onMessage(const QByteArray& data) {

    qDebug() << "Garmin: GdfiMessageCallback onMessage called  with data  " << data;
}

MlrCommunicator::MlrCommunicator(quint8 handle,
                                 int maxPacketSize,
                                 QSharedPointer<MlrMessageSender> sender,
                                 QSharedPointer<MlrMessageReceiver> receiver,
                                 QObject* parent)
    : QObject(parent)
    , m_sender(sender)
    , m_receiver(receiver)
{
    connect (&retransmissionTimer,&QTimer::timeout,this,&MlrCommunicator::onRetransmissionTimeout);
    retransmissionTimer.setInterval(INITIAL_RETRANSMISSION_TIMEOUT_MS);

    connect (&ackTimer,&QTimer::timeout,this,&MlrCommunicator::sendAckPacket);
    ackTimer.setInterval(ACK_TIMEOUT_MS);

    m_state.handle = handle;
    m_state.maxPacketSize = maxPacketSize;


    if (m_receiver) {
        connect(m_receiver.data(), &MlrMessageReceiver::receiverError,
                this, [&](const QString& err){
                    qDebug() << "Garmin: Receiver failed to handle MLR data: " << err;
                });
    }

    qDebug() << "Garmin: Creating MLR communicator for handle" <<handle;
}

MlrCommunicator::~MlrCommunicator() {
    close();
}


void MlrCommunicator::setMaxPacketSize(int maxPacketSize) {
    ////QMutexLocker lock(&m_mutex);
    m_state.maxPacketSize = maxPacketSize;
    qDebug()<< "Garmin: MLR max packet size set to " << maxPacketSize;
}

Result<void> MlrCommunicator::start() {
    qDebug() << "Garmin: Starting MlrCommunicator";
    ////QMutexLocker lock(&m_mutex);
    if (m_running) return Result<void>::isOk();
    m_running = true;
    retransmissionTimer.start();
    qDebug() << "Garmin: MlrCommunicator started";

    return Result<void>::isOk();
}

void MlrCommunicator::sendMessage(const QString& taskName, const QByteArray& message) {
    if (message.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "Garmin: Empty Message  in MLR Commnicator";
        return;
    }



    qDebug() << Q_FUNC_INFO << "Garmin: Queuing MLR message for " << taskName <<  "(" << message.size() <<" bytes)";

    const int maxDataSize = qMax(0, m_state.maxPacketSize - 2);

    int remaining = message.size();
    int i = 0;
    if (remaining > maxDataSize) {
        int position = 0;
        while (remaining > 0) {
            const int chunk = qMin(remaining, maxDataSize);
            Fragment f;
            f.taskName = taskName;
            f.num = i;
            f.data = message.mid(position, chunk);
            f.reqNum=0;
            m_state.fragmentQueue.enqueue(f);

            position += f.data.size();
            remaining -= f.data.size();
            i++;
        }
    }
    else {
        Fragment f;
        f.taskName=taskName;
        f.num=0;
        f.data=message;
        f.reqNum=0;
        m_state.fragmentQueue.enqueue(f);
    }


    runProtocolOnce();
}

void MlrCommunicator::onPacketReceived(const QByteArray& packet) {
    qDebug() << Q_FUNC_INFO << "Garmin: received packet " << packet.toHex();
    if (packet.size() < 2) {
        qDebug() << Q_FUNC_INFO << "Garmin: MLR packet too short.";
        return;
    }

    const quint8 byte0 = quint8(packet[0]);
    const quint8 byte1 = quint8(packet[1]);

    if ((byte0 & MLR_FLAG_MASK) == 0) {
        qDebug() << Q_FUNC_INFO << "Garmin:Received non-MLR packet";
        return;
    }

    const quint8 packetHandle = (byte0 & HANDLE_MASK) >> HANDLE_SHIFT;
    const quint8 reqNum = quint8(((byte0 & REQ_NUM_MASK) << 2) | ((byte1 >> 6) & 0x03));
    const quint8 seqNum = quint8(byte1 & SEQ_NUM_MASK);


    if (packetHandle != (m_state.handle & 0x07)) {
        qDebug() << Q_FUNC_INFO << "Garmin:MLR packet for wrong handle: expected " << (m_state.handle & 0x07) << ", got " << packetHandle;
        return;
    }

    qDebug() << Q_FUNC_INFO << "Garmin: MLR packet received: reqNum=" << reqNum <<", seqNum=" << seqNum << ", dataLen=" << packet.size() - 2;

    // Process ACK if reqNum changed
    if (reqNum != m_state.lastRcvAck) {
         processAck(reqNum);
    }

    // Process data if any
    if (packet.size() > 2) {
        if (seqNum == m_state.nextRcvSeq) {
            // in-sequence
            const QByteArray data = packet.mid(2);
            if (m_receiver) {
                m_receiver.data()->onDataReceived(data);
            }
            m_state.nextRcvSeq = quint8((m_state.nextRcvSeq + 1) % (MAX_SEQ_NUM + 1));
            scheduleAck();
        } else {
            qDebug() << Q_FUNC_INFO << "Gamin:  Out-of-sequence packet - expected " << m_state.nextRcvSeq <<", got " << seqNum;
            // re-send expected ack
            sendAckPacket();
        }
    }
    runProtocolOnce();

}

void MlrCommunicator::close() {
    //QMutexLocker lock(&m_mutex);
    if (!m_running) return;
    qDebug() << "Garmin: Closing MLR communicator";
    m_running = false;
    retransmissionTimer.stop();
}



void MlrCommunicator::onRetransmissionTimeout() {
    qDebug() << Q_FUNC_INFO << "Garmin: Retransmission timeout expired";
    // Backoff retransmission timeout and reduce the maximum unacked
    m_state.retransmissionTimeoutMs = std::min(m_state.retransmissionTimeoutMs * 2, MAX_RETRANSMISSION_TIMEOUT_MS);
    m_state.maxNumUnackedSend = std::max(1, m_state.maxNumUnackedSend / 2);


    for (int i = m_state.lastRcvAck; i != m_state.nextSendSeq; i = (i + 1) % (MAX_SEQ_NUM + 1)) {
        qDebug() << Q_FUNC_INFO << "Re-sending fragment " <<  i;
        if (m_state.sentFragments.contains(i))
        {
            Fragment fragment = m_state.sentFragments[i];
            QByteArray packet = createPacket(fragment.reqNum, i, fragment.data);
            if (m_sender) m_sender->sendPacket("retransmission " + fragment.taskName + " (" + fragment.num + ")", packet);
        } else qDebug() << Q_FUNC_INFO << "Garmin: Attempting to re-send null fragment at index " << i;
    }
    startRetransmissionTimer();
}

// =============================================================================
// Static helpers (1:1 logic)
// =============================================================================
int MlrCommunicator::seqDiff(quint8 a, quint8 b) {
    const int mod = int(MAX_SEQ_NUM) + 1;
    return ( (int(a) - int(b) + mod) % mod );
}

QByteArray MlrCommunicator::createPacket(quint8 reqNum, quint8 seqNum, const QByteArray& data) {
    QByteArray packet;

    const quint8 byte0 = quint8(MLR_FLAG_MASK |
                                ((m_state.handle & 0x07) << HANDLE_SHIFT) |
                                ((reqNum >> 2) & REQ_NUM_MASK));
    const quint8 byte1 = quint8(((reqNum & 0x03) << 6) | (seqNum & SEQ_NUM_MASK));

    packet.append(char(byte0));
    packet.append(char(byte1));
    packet.append(data);
    return packet;
}

void MlrCommunicator::processAck(quint8 reqNum) {

    qDebug() << Q_FUNC_INFO << "Garmin: MLRCommunicator processing Ack";
    const int numAcked = seqDiff(reqNum, m_state.lastRcvAck);

    retransmissionTimer.stop();

    quint8 i = m_state.lastRcvAck;
    while (i != reqNum) {
        if (m_state.sentFragments.contains(i)) m_state.sentFragments.remove(i);
        i = quint8((i + 1) % (MAX_SEQ_NUM + 1));
    }
    m_state.lastRcvAck = reqNum;

    // Restart retransmit timer if still unacked
    if (m_state.lastRcvAck != m_state.nextSendSeq) {
        qDebug() << Q_FUNC_INFO <<  "Garmin: Restarting Ack Timer";
        startRetransmissionTimer();
    }

    Q_UNUSED(numAcked);
}

void MlrCommunicator::scheduleAck() {
    ackTimer.stop();
    const int numRcvdUnacked = seqDiff(m_state.nextRcvSeq, m_state.lastSendAck);
    if (numRcvdUnacked >= ACK_TRIGGER_THRESHOLD) {
        sendAckPacket();
    } else {
        ackTimer.start();
    }
}

// =============================================================================
// Sending helpers
// =============================================================================


void MlrCommunicator::sendAckPacket() {
    ackTimer.stop();
    const QByteArray pkt = createPacket(m_state.nextRcvSeq, 0, QByteArray());
    const QString task = QStringLiteral("ack reqNum=%1").arg(m_state.nextRcvSeq);
    if (m_sender) m_sender->sendPacket(task,pkt);
    m_state.lastSendAck = m_state.nextRcvSeq;
    return;
}

// =============================================================================
// Protocol core (run_protocol)
// =============================================================================
void MlrCommunicator::runProtocolOnce() {
    Fragment frag;
    quint8 seq = 0;
    quint8 req = 0;
    int numSentUnacked;


    numSentUnacked = seqDiff(m_state.nextSendSeq, m_state.lastRcvAck);
    if (numSentUnacked >= m_state.maxNumUnackedSend) {
        qDebug() << Q_FUNC_INFO << "Garmin: Cannot send more packets, " << numSentUnacked <<" unacked, max " << m_state.maxNumUnackedSend;
        return;
    }
    if (m_state.fragmentQueue.isEmpty()) {
        return;
    }

    frag = m_state.fragmentQueue.dequeue();
    req = m_state.nextRcvSeq;
    seq = m_state.nextSendSeq;
    Fragment fragmentWithReqNum;
    fragmentWithReqNum.taskName=frag.taskName;
    fragmentWithReqNum.num=frag.num;
    fragmentWithReqNum.data=frag.data;
    fragmentWithReqNum.reqNum= m_state.nextRcvSeq;
    const QByteArray pkt = createPacket(m_state.nextRcvSeq,m_state.nextSendSeq,frag.data);
    const QString taskName = QStringLiteral("%1 (%2)").arg(frag.taskName).arg(frag.num);
    if (m_sender) m_sender->sendPacket(taskName, pkt);

    m_state.sentFragments.insert(seq,fragmentWithReqNum);
    m_state.nextSendSeq = quint8((m_state.nextSendSeq + 1) % (MAX_SEQ_NUM + 1));

    if (numSentUnacked == 0) {
        startRetransmissionTimer();
    }
    qDebug() << Q_FUNC_INFO << "Garmin: Sent MLR packet: seqNum=" << seq << ", dataLen=" << pkt.size() - 2;
}


void MlrCommunicator::startRetransmissionTimer() {
    retransmissionTimer.start();
}
