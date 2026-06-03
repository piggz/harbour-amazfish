#include "asyncmessagehandler.h"


#include <QtCore/QProcessEnvironment>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include "garmintypes.h"

AsyncMessageHandler::AsyncMessageHandler(QObject* parent)
    : QObject(parent)
    , mInitializationComplete(false)
    ,mPairingDetected(false)
    , mSendSemaphore(1)
{
    mCommunicator.reset();
    mNotificationHandler.reset();

    mMessageQueue.clear();
    mPendingProtobufChunks.clear();


    // DataTransferHandler::new()
    // Assumes DataTransferHandler has a default constructor or static create().
    mDataTransferHandler = QSharedPointer<DataTransferHandler>::create();

    // -----------------------------
    // Start queue processor
    // - else sleep 10ms
    // We'll tick every 10ms and enforce the 50ms spacing by holding semaphore
    // and releasing it after 50ms.
    // -----------------------------
    m_queueTimer.setParent(this);
    m_queueTimer.setInterval(10);
    connect(&m_queueTimer, &QTimer::timeout, this, &AsyncMessageHandler::processQueueTick);
    m_queueTimer.start();
}

void AsyncMessageHandler::setCommunicator(const QSharedPointer<CommunicatorV2>& comm) {
    QMutexLocker lock(&mMutex);
    mCommunicator = comm;
}

void AsyncMessageHandler::setNotificationHandler(const QSharedPointer<GarminNotificationHandler>& handler) {
    QMutexLocker lock(&mMutex);
    mNotificationHandler = handler;

    // Rust also sets watchdog reference from handler.watchdog.clone().
    // Assumes GarminNotificationHandler exposes watchdog() -> QSharedPointer<WatchdogManager>.
    // If your API differs, adjust here.
    // m_watchdog = handler ? handler->watchdog() : nullptr;
}

/*
void AsyncMessageHandler::setCalendarManager(const QSharedPointer<CalendarManager>& manager) {
    QMutexLocker lock(&mMutex);
    mCalendarManager = manager;
}
*/

Result<void> AsyncMessageHandler::sendResponse(const QByteArray& response) {
    // Rust: push_back(response.to_vec()) and return Ok(())
    QMutexLocker lock(&mMutex);
    mMessageQueue.enqueue(response);
    return Result<void>::isOk();
}

void AsyncMessageHandler::processQueueTick() {
    // - pop_front
    // - acquire semaphore permit (only one send at a time)
    // - send if communicator exists
    // - sleep 50ms between messages
    // - if no message => do nothing (timer already 10ms)
    QByteArray msg;
    QSharedPointer<CommunicatorV2> comm;

    {
        QMutexLocker lock(&mMutex);
        if (mMessageQueue.isEmpty()) return;

        // If we cannot acquire, we're in the 50ms cooldown period
        if (!mSendSemaphore.tryAcquire(1)) return;

        msg = mMessageQueue.dequeue();
        comm = mCommunicator;
    }

    if (comm) {
        // Rust: communicator.send_message("queued_message", &msg).await
        bool r = comm->sendMessage(QStringLiteral("queued_message"), msg);
        if (!r) {
            qDebug() << Q_FUNC_INFO << "Garmin: QUEUE Failed to send message.";
        }
    }

    // Release semaphore after 50ms (Rust: sleep(50ms) after send)
    QTimer::singleShot(50, this, [this]() {
        mSendSemaphore.release(1);
    });
}

quint16 AsyncMessageHandler::computeCrc16A001(const QByteArray& data) {
    // Same algorithm as your Rust snippet uses in send_protobuf_chunk:
    // crc ^= byte; for 8 bits: if (crc&1) crc=(crc>>1)^0xA001 else crc>>=1
    quint16 crc = 0;
    for (auto ch : data) {
        crc ^= quint16(quint8(ch));
        for (int i = 0; i < 8; ++i) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

Result<void> AsyncMessageHandler::sendProtobufResponse(const QByteArray& message) {
    // Rust: if message.len() < 18 -> send as-is (enqueue)
    if (message.size() < 18) {
        return sendResponse(message);
    }

    // Rust: message_id = u16::from_le_bytes([message[2], message[3]])
    const quint16 messageId = u16le(message, 2);
    if (messageId != 0x13B4) {
        // Not ProtobufResponse
        return sendResponse(message);
    }

    // request_id = [4,5]
    const quint16 requestId = u16le(message, 4);

    // protobuf payload = message[18..len-2]
    if (message.size() < 20) {
        return sendResponse(message);
    }
    const QByteArray protobufPayload = message.mid(18, message.size() - 18 - 2);

    if (protobufPayload.size() <= MAX_PROTOBUF_CHUNK_SIZE) {
        emit logInfo(QStringLiteral("✅ Message fits in one chunk, sending directly"));
        return sendResponse(message);
    }

    emit logInfo(QStringLiteral("📦 Message too large, chunking into %1-byte chunks").arg(MAX_PROTOBUF_CHUNK_SIZE));

    {
        QMutexLocker lock(&mMutex);
        PendingProtobufChunk chunk;
        chunk.completePayload = protobufPayload;
        chunk.totalLength = protobufPayload.size();
        chunk.messageType = messageId;
        chunk.requestId = requestId;
        mPendingProtobufChunks.insert(requestId, chunk);
        emit logInfo(QStringLiteral("📦 Stored pending chunk for request ID %1").arg(requestId));
    }

    // first chunk [0..MAX]
    const QByteArray firstChunk = protobufPayload.left(MAX_PROTOBUF_CHUNK_SIZE);

    return sendProtobufChunk(requestId, 0, firstChunk, protobufPayload.size());
}

Result<void> AsyncMessageHandler::sendProtobufChunk(quint16 requestId,
                                                   quint32 dataOffset,
                                                   const QByteArray& chunkData,
                                                   int totalLength)
{
    // Rust builds a full GFDI message for ProtobufResponse chunk:
    // [size placeholder 2]
    // [5044 u16]
    // [requestId u16]
    // [dataOffset u32]
    // [totalLen u32]
    // [chunkLen u32]
    // [chunk bytes]
    // [packet size fill]
    // [crc16]
    QByteArray msg;
    msg.reserve(2 + 2 + 2 + 4 + 4 + 4 + chunkData.size() + 2);

    // size placeholder
    msg.append(char(0));
    msg.append(char(0));

    // msg id 5044
    writeU16le(msg, 5044);

    // request id
    writeU16le(msg, requestId);

    // data offset
    writeU32le(msg, dataOffset);

    // total len
    writeU32le(msg, quint32(totalLength));

    // chunk len
    writeU32le(msg, quint32(chunkData.size()));

    // payload chunk
    msg.append(chunkData);

    // fill packet size = message.len() + 2
    const quint16 packetSize = quint16(msg.size() + 2);
    msg[0] = char(packetSize & 0xFF);
    msg[1] = char((packetSize >> 8) & 0xFF);

    // crc16
    const quint16 crc = computeCrc16A001(msg);
    writeU16le(msg, crc);

    emit logDebug(QStringLiteral("📤 Chunk message size: %1 bytes").arg(msg.size()));

    // Rust: send_response(&message).await => enqueue
    return sendResponse(msg);
}

Result<void> AsyncMessageHandler::handleProtobufChunkAck(quint16 requestId, quint32 dataOffset) {
    std::optional<PendingProtobufChunk> pendingChunk;

    {
        QMutexLocker lock(&mMutex);
        auto it = mPendingProtobufChunks.find(requestId);
        if (it != mPendingProtobufChunks.end()) {
            pendingChunk = it.value(); // cloned()
        }
    }

    if (!pendingChunk.has_value()) {
        emit logWarn(QStringLiteral("ℹ️ No pending chunk found for request ID %1").arg(requestId));
        return Result<void>::isOk();
    }

    const PendingProtobufChunk chunkInfo = *pendingChunk;

    const int nextOffset = int(dataOffset) + MAX_PROTOBUF_CHUNK_SIZE;

    const int remainingSaturating = (chunkInfo.totalLength > nextOffset)
                                        ? (chunkInfo.totalLength - nextOffset)
                                        : 0;

    emit logDebug(QStringLiteral("📦 Checking for next chunk: request=%1 currentOffset=%2 nextOffset=%3 total=%4 remaining=%5")
                  .arg(requestId)
                  .arg(dataOffset)
                  .arg(nextOffset)
                  .arg(chunkInfo.totalLength)
                  .arg(remainingSaturating));

    if (nextOffset < chunkInfo.totalLength) {
        const int remaining = chunkInfo.totalLength - nextOffset;
        const int chunkSize = qMin(remaining, MAX_PROTOBUF_CHUNK_SIZE);

        if (nextOffset + chunkSize > chunkInfo.completePayload.size()) {
            emit logError(QStringLiteral("❌ ERROR: Chunk boundary exceeds payload! offset=%1 size=%2 total=%3")
                          .arg(nextOffset)
                          .arg(chunkSize)
                          .arg(chunkInfo.completePayload.size()));
            return Result<void>::isOk(); // Rust returns Ok(())
        }

        const QByteArray nextChunk = chunkInfo.completePayload.mid(nextOffset, chunkSize);

        return sendProtobufChunk(requestId,
                                 quint32(nextOffset),
                                 nextChunk,
                                 chunkInfo.totalLength);
    }

    // Done: remove pending
    {
        QMutexLocker lock(&mMutex);
        mPendingProtobufChunks.remove(requestId);
    }
    emit logInfo(QStringLiteral("✅ All chunks sent for request ID %1").arg(requestId));
    emit logDebug(QStringLiteral("🎉 Transfer complete! Removed from pending chunks."));

    return Result<void>::isOk();
}
