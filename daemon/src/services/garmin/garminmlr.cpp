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
    m_sendChar->writeValue(packet,&errorMsg);
    if (!errorMsg.isEmpty()) qDebug() << "Garmin: MlrMEssagesender send packet failed with error " <<errorMsg;
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
    qDebug() << "Garmin:MLR received data: " << data.length() << " bytes";

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
        qDebug() << "Garmin: Warning: MLR decoded empty message";
        return;
    }

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
    m_clock.start();

    //QMutexLocker lock(&m_mutex);
    m_state.handle = handle;
    m_state.maxPacketSize = maxPacketSize;

    m_timer.setInterval(100);
    connect(&m_timer, &QTimer::timeout, this, &MlrCommunicator::onTick);

    if (m_receiver) {
        connect(m_receiver.data(), &MlrMessageReceiver::receiverError,
                this, [&](const QString& err){
                    qDebug() << "Garmin: Receiver failed to handle MLR data: " << err;
                });
    }

    qDebug() << "Garmin: Creating MLR communicator for handle" <<handle;
}

MlrCommunicator::~MlrCommunicator() {
    // Rust Drop sends shutdown; here stop timer.
    close();
}

// =============================================================================
// time helper
// =============================================================================
qint64 MlrCommunicator::nowMs() const {
    return m_clock.elapsed();
}

// =============================================================================
// Public API
// =============================================================================

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
    m_timer.start();
    qDebug() << "Garmin: MlrCommunicator started";

    return Result<void>::isOk();
}

Result<void> MlrCommunicator::sendMessage(const QString& taskName, const QByteArray& message) {
    if (message.isEmpty()) {
        qDebug() << "Garmin: Empty Message  in MLR Commnicator";
        return Result<void>::err(GarminError::emptyMessage());
    }

    {
        QMutexLocker lock(&m_mutex);
        if (m_state.paused) {
            qDebug() << "Garmin: MLR is paused - rejecting send_message for " << taskName;
            return Result<void>::err(GarminError::bluetoothError(QStringLiteral("MLR is paused during reconnection")));
        }

        qDebug() << "Garmin: Queuing MLR message for " << taskName <<  "(" << message.size() <<" bytes)";

        const int maxDataSize = qMax(0, m_state.maxPacketSize - 2);

        int remaining = message.size();
        int position = 0;
        int fragmentNum = 0;

        while (remaining > 0) {
            const int chunk = qMin(remaining, maxDataSize);
            Fragment f;
            f.taskName = taskName;
            f.num = fragmentNum;
            f.data = message.mid(position, chunk);
            m_state.fragmentQueue.enqueue(f);

            position += chunk;
            remaining -= chunk;
            fragmentNum += 1;
        }
        lock.unlock();
    }

    // Rust drops lock then run_protocol().await
    return runProtocolOnce();
}

Result<void> MlrCommunicator::onPacketReceived(const QByteArray& packet) {
    if (packet.size() < 2) {
        return Result<void>::err(GarminError::packetTooShort(packet.size()));
    }

    const quint8 byte0 = quint8(packet[0]);
    const quint8 byte1 = quint8(packet[1]);

    if ((byte0 & MLR_FLAG_MASK) == 0) {
        return Result<void>::err(GarminError::notMlrPacket());
    }

    const quint8 packetHandle = (byte0 & HANDLE_MASK) >> HANDLE_SHIFT;
    const quint8 reqNum = quint8(((byte0 & REQ_NUM_MASK) << 2) | ((byte1 >> 6) & 0x03));
    const quint8 seqNum = quint8(byte1 & SEQ_NUM_MASK);

    {
        QMutexLocker lock(&m_mutex);

        if (packetHandle != (m_state.handle & 0x07)) {
            return Result<void>::err(GarminError::invalidHandle(m_state.handle & 0x07, packetHandle));
        }

        qDebug() << "Garmin: MLR packet received: reqNum=" << reqNum <<", seqNum=" << seqNum << ", dataLen=" << packet.size() - 2;

        // Process ACK if reqNum changed
        if (reqNum != m_state.lastRcvAck) {
            qDebug() << "Garmin: Processicng Ackk, last Acc " << m_state.lastRcvAck <<", current Ack = " << reqNum ;
            auto r = processAck(m_state, reqNum);
            if (!r.ok) return r;
        }

        // Process data if any
        if (packet.size() > 2) {
            if (seqNum == m_state.nextRcvSeq) {
                // in-sequence
                const QByteArray data = packet.mid(2);
                m_state.nextRcvSeq = quint8((m_state.nextRcvSeq + 1) % (MAX_SEQ_NUM + 1));
                qDebug() << "Gamin: Scheduling ACK";
                scheduleAck(m_state, nowMs());

                // deliver to receiver *after* releasing lock (matches Rust drop(state))
                if (m_receiver) {
                    QMetaObject::invokeMethod(m_receiver.data(), "onDataReceived",
                                              Qt::QueuedConnection,
                                              Q_ARG(QByteArray, data));
                }
            } else {
                qDebug() << "Gamin:  Out-of-sequence packet - expected " << m_state.nextRcvSeq <<", got " << seqNum;
                // re-send expected ack
                auto ackRes = sendAckPacketLocked(m_state);
                if (!ackRes.ok) return ackRes;
            }
        }
    }

    // Rust runs protocol up to 10 times to drain queue if window opens
    for (int i=0; i<10; ++i) {
        auto r = runProtocolOnce();
        if (!r.ok) {
            qDebug() << "Garmin: Protocol run failed: " << r.error.toString();
            break;
        }

        QMutexLocker lock(&m_mutex);
        if (m_state.fragmentQueue.isEmpty()) break;

        const int unacked = seqDiff(m_state.nextSendSeq, m_state.lastRcvAck);
        if (unacked >= m_state.maxNumUnackedSend) {
            qDebug() << "Garmin: Send window full, will resume after next ACK";
            break;
        }
        lock.unlock();
    }

    return Result<void>::isOk();
}

void MlrCommunicator::pause() {
    //QMutexLocker lock(&m_mutex);
    qDebug() << "Garmin: Pausing MLR communicator";
    m_state.paused = true;
}

void MlrCommunicator::resume() {
    //QMutexLocker lock(&m_mutex);
    qDebug() << Q_FUNC_INFO << "Garmin: Resuming MLR communicator";
    m_state.paused = false;
}

void MlrCommunicator::clearAndPause() {
    //QMutexLocker lock(&m_mutex);
    qDebug() << Q_FUNC_INFO << "Garmin: Clearing and pausing MLR state due to disconnection";

    m_state.paused = true;

    for (auto& opt : m_state.sentFragments) opt.reset();
    m_state.fragmentQueue.clear();

    // Reset sequence numbers but keep nextSendSeq
    m_state.lastRcvAck = m_state.nextSendSeq;

    m_state.lastAckTimeMs.reset();
    m_state.lastRetransmitTimeMs.reset();

    m_state.retransmissionTimeoutMs = INITIAL_RETRANSMISSION_TIMEOUT_MS;
    m_state.maxNumUnackedSend = INITIAL_MAX_UNACKED_SEND;

    qDebug() << "Garmin: MLR state cleared and paused";
}

void MlrCommunicator::close() {
    //QMutexLocker lock(&m_mutex);
    if (!m_running) return;
    qDebug() << "Garmin: Closing MLR communicator";
    m_running = false;
    m_timer.stop();
}

// =============================================================================
// Timer tick (replaces Tokio background task)
// =============================================================================
void MlrCommunicator::onTick() {
    auto a = checkAckTimeout();
    if (!a.ok) qDebug() << "Garmin: ACK timeout check failed: " << a.error.toString();

    auto r = checkRetransmitTimeout();
    if (!r.ok) {
        qDebug() << "Garmin: Retransmit timeout check failed: " << r.error.toString();
        clearStateLocked(m_state);
    }
}

// =============================================================================
// Static helpers (1:1 logic)
// =============================================================================
int MlrCommunicator::seqDiff(quint8 a, quint8 b) {
    const int mod = int(MAX_SEQ_NUM) + 1;
    return ( (int(a) - int(b) + mod) % mod );
}

QByteArray MlrCommunicator::createPacket(const State& st, quint8 reqNum, quint8 seqNum, const QByteArray& data) {
    QByteArray packet;
    packet.reserve(2 + data.size());

    const quint8 byte0 = quint8(MLR_FLAG_MASK |
                                ((st.handle & 0x07) << HANDLE_SHIFT) |
                                ((reqNum >> 2) & REQ_NUM_MASK));
    const quint8 byte1 = quint8(((reqNum & 0x03) << 6) | (seqNum & SEQ_NUM_MASK));

    packet.append(char(byte0));
    packet.append(char(byte1));
    packet.append(data);
    return packet;
}

Result<void> MlrCommunicator::processAck(State& st, quint8 reqNum) {

    qDebug() << "Garmin: MLRCommunicator processing Ack";
    const int numAcked = seqDiff(reqNum, st.lastRcvAck);

    // Rust clears retransmit timer
    st.lastRetransmitTimeMs.reset();

    quint8 i = st.lastRcvAck;
    while (i != reqNum) {
        st.sentFragments[int(i)].reset();
        i = quint8((i + 1) % (MAX_SEQ_NUM + 1));
    }
    st.lastRcvAck = reqNum;

    // Restart retransmit timer if still unacked
    if (st.lastRcvAck != st.nextSendSeq) {
        qDebug() << "Garmin: Restarting Ack Timer";
        st.lastRetransmitTimeMs = 0; // will be overwritten by caller with nowMs; kept simple
    }

    Q_UNUSED(numAcked);
    return Result<void>::isOk();
}

void MlrCommunicator::scheduleAck(State& st, qint64 nowMs) {
    const int numRcvdUnacked = seqDiff(st.nextRcvSeq, st.lastSendAck);
    if (numRcvdUnacked >= ACK_TRIGGER_THRESHOLD) {
        st.lastSendAck = st.nextRcvSeq;
        st.lastAckTimeMs.reset();
    } else {
        st.lastAckTimeMs = nowMs;
    }
}

// =============================================================================
// Sending helpers
// =============================================================================
Result<void> MlrCommunicator::awaitSend(const QString& taskName, const QByteArray& packet) {
    if (!m_sender) {
        return Result<void>::err(GarminError::bluetoothError(QStringLiteral("MessageSender not set")));
    }
    Result<void> outcome = Result<void>::isOk();

    // invoke sender (queued) then wait (models .await)
    QMetaObject::invokeMethod(m_sender.data(), "sendPacket", Qt::QueuedConnection,
                              Q_ARG(QString, taskName),
                              Q_ARG(QByteArray, packet));

    return outcome;
}

Result<void> MlrCommunicator::sendAckPacketLocked(const State& st) {
    const QByteArray pkt = createPacket(st, st.nextRcvSeq, 0, QByteArray());
    const QString task = QStringLiteral("ack reqNum=%1").arg(st.nextRcvSeq);
    auto r = awaitSend(task, pkt);
    if (r.ok) emit qDebug() << "Garmin:Sent ACK packet: reqNum=" <<st.nextRcvSeq;
    return r;
}

// =============================================================================
// Protocol core (run_protocol)
// =============================================================================
Result<void> MlrCommunicator::runProtocolOnce() {
    Fragment frag;
    quint8 seq = 0;
    quint8 req = 0;
    int numSentUnacked = 0;

    {

        if (m_state.paused) return Result<void>::isOk();

        numSentUnacked = seqDiff(m_state.nextSendSeq, m_state.lastRcvAck);
        if (numSentUnacked >= m_state.maxNumUnackedSend) {
            return Result<void>::isOk();
        }
        if (m_state.fragmentQueue.isEmpty()) {
            return Result<void>::isOk();
        }

        frag = m_state.fragmentQueue.dequeue();
        req = m_state.nextRcvSeq;
        seq = m_state.nextSendSeq;

        m_state.sentFragments[int(seq)] = frag;
        m_state.nextSendSeq = quint8((m_state.nextSendSeq + 1) % (MAX_SEQ_NUM + 1));

        if (numSentUnacked == 0) {
            m_state.lastRetransmitTimeMs = nowMs();
        }
    }

    const QByteArray pkt = createPacket(m_state, req, seq, frag.data);
    const QString taskName = QStringLiteral("%1 (%2)").arg(frag.taskName).arg(frag.num);

    auto r = awaitSend(taskName, pkt);
    if (!r.ok) return r;

    qDebug() << "Garmin: Sent MLR packet: seqNum=" << seq << ", dataLen=" << pkt.size() - 2;
    return Result<void>::isOk();
}

// =============================================================================
// check_ack_timeout
// =============================================================================
Result<void> MlrCommunicator::checkAckTimeout() {
    bool shouldSend = false;
    quint8 nextRcvSeq = 0;
    QByteArray pkt;

    {
        if (m_state.paused) return Result<void>::isOk();
        if (!m_state.lastAckTimeMs.has_value()) return Result<void>::isOk();

        const qint64 elapsed = nowMs() - *m_state.lastAckTimeMs;
        if (elapsed < ACK_TIMEOUT_MS) return Result<void>::isOk();

        nextRcvSeq = m_state.nextRcvSeq;
        pkt = createPacket(m_state, nextRcvSeq, 0, QByteArray());

        m_state.lastSendAck = nextRcvSeq;
        m_state.lastAckTimeMs.reset();

        shouldSend = true;
    }

    if (shouldSend) {
        const QString task = QStringLiteral("ack reqNum=%1").arg(nextRcvSeq);
        auto r = awaitSend(task, pkt);
        if (!r.ok) return r;
        qDebug() << "Garmin: Sent ACK packet after timeout: reqNum=%1" << nextRcvSeq;
    }

    return Result<void>::isOk();
}

// =============================================================================
// clear_state (used after retransmit errors)
// =============================================================================
void MlrCommunicator::clearStateLocked(State& st) {
    for (auto& opt : st.sentFragments) opt.reset();
    st.fragmentQueue.clear();
    st.lastAckTimeMs.reset();
    st.lastRetransmitTimeMs.reset();
    st.retransmissionTimeoutMs = INITIAL_RETRANSMISSION_TIMEOUT_MS;
    st.maxNumUnackedSend = INITIAL_MAX_UNACKED_SEND;
}

// =============================================================================
// check_retransmit_timeout
// =============================================================================
Result<void> MlrCommunicator::checkRetransmitTimeout() {
    struct ToSend { QString task; QByteArray pkt; quint8 seq; };
    QVector<ToSend> packets;
    bool hadPackets = false;

    {
        if (m_state.paused) return Result<void>::isOk();
        if (!m_state.lastRetransmitTimeMs.has_value()) return Result<void>::isOk();

        const qint64 elapsed = nowMs() - *m_state.lastRetransmitTimeMs;
        if (elapsed < m_state.retransmissionTimeoutMs) return Result<void>::isOk();

        // backoff (Rust: timeout*=2 capped; maxUnacked=ceil/2 min 1) [1](https://computacenter-my.sharepoint.com/personal/thomas_michel_computacenter_com/Documents/Microsoft%20Copilot%20Chat%20Files/mlr.rs)
        m_state.retransmissionTimeoutMs = qMin(m_state.retransmissionTimeoutMs * 2, MAX_RETRANSMISSION_TIMEOUT_MS);
        m_state.maxNumUnackedSend = qMax(1, (m_state.maxNumUnackedSend + 1) / 2);

        quint8 i = m_state.lastRcvAck;
        while (i != m_state.nextSendSeq) {
            if (!m_state.sentFragments[int(i)].has_value()) {
                return Result<void>::err(GarminError::ioError(QStringLiteral("Attempting to re-send null fragment")));
            }
            const Fragment& f = *m_state.sentFragments[int(i)];
            QByteArray pkt = createPacket(m_state, m_state.nextRcvSeq, i, f.data);
            QString task = QStringLiteral("retransmission %1 (%2)").arg(f.taskName).arg(f.num);
            packets.push_back({task, pkt, i});
            i = quint8((i + 1) % (MAX_SEQ_NUM + 1));
        }

        hadPackets = !packets.isEmpty();
    }

    int sentCount = 0;
    for (const auto& p : packets) {
        auto r = awaitSend(p.task, p.pkt);
        if (r.ok) {
            qDebug() << "Garmin:Re-sent fragment %1" << p.seq;
            sentCount++;
        } else {
            const QString errMsg = r.error.toString();
            if (errMsg.contains(QStringLiteral("Not connected"), Qt::CaseInsensitive)) {
                qDebug() << "Garmin: Connection lost during retransmission - clearing MLR state";
                for (auto& opt : m_state.sentFragments) opt.reset();
                m_state.fragmentQueue.clear();
                m_state.lastRetransmitTimeMs.reset();
                return r;
            }
            return r;
        }
    }

    {
        if (sentCount > 0) {
            m_state.lastRetransmitTimeMs = nowMs();
        } else if (hadPackets) {
            m_state.lastRetransmitTimeMs.reset();
            qDebug() << "Garmin: Retransmission timeout with no valid fragments sent - clearing timer";
        }
    }

    return Result<void>::isOk();
}

