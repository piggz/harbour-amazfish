#include "asyncmessagehandler.h"


#include <QtCore/QProcessEnvironment>
#include <QtCore/QString>
#include <QtCore/QDebug>

#include "garmintypes.h"

static inline QString transferStatusName(quint8 transferStatus)
{
    switch (transferStatus) {
    case 0: return QStringLiteral("OK");
    case 1: return QStringLiteral("RESEND");
    case 2: return QStringLiteral("ABORT");
    case 3: return QStringLiteral("CRC_MISMATCH");
    case 4: return QStringLiteral("OFFSET_MISMATCH");
    default: return QStringLiteral("UNKNOWN");
    }
}

static inline QString responseMessageName(quint16 originalMsgId)
{
    switch (originalMsgId) {
    case 0x13A9: return QStringLiteral("NotificationUpdate");
    case 0x13AA: return QStringLiteral("NotificationControl");
    case 0x13AC: return QStringLiteral("NotificationSubscription");
    case 0x13A0: return QStringLiteral("DeviceInformation");
    case 0x13BA: return QStringLiteral("Configuration");
    default:     return QStringLiteral("Unknown");

    }
}

AsyncMessageHandler::AsyncMessageHandler(QObject* parent)
    : QObject(parent)
    , mInitializationComplete(false)
    ,mPairingDetected(false)
    , mSendSemaphore(1)
{
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

void AsyncMessageHandler::setCommunicator(CommunicatorV2* comm) {
    mCommunicator = comm;
}

void AsyncMessageHandler::setNotificationHandler(const QSharedPointer<GarminNotificationHandler>& handler) {
    mNotificationHandler = handler;
}

/*
void AsyncMessageHandler::setCalendarManager(const QSharedPointer<CalendarManager>& manager) {
    QMutexLocker lock(&mMutex);
    mCalendarManager = manager;
}
*/

bool AsyncMessageHandler::sendResponse(const QByteArray& response) {
    // Rust: push_back(response.to_vec()) and return Ok(())
    mMessageQueue.enqueue(response);
    return true;
}

void AsyncMessageHandler::processQueueTick() {
    // - pop_front
    // - acquire semaphore permit (only one send at a time)
    // - send if communicator exists
    // - sleep 50ms between messages
    // - if no message => do nothing (timer already 10ms)
    QByteArray msg;
    CommunicatorV2* comm;

    {
        if (mMessageQueue.isEmpty()) return;

        // If we cannot acquire, we're in the 50ms cooldown period
        if (!mSendSemaphore.tryAcquire(1)) return;

        msg = mMessageQueue.dequeue();
        comm = mCommunicator;
    }

    if (comm) {
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

bool AsyncMessageHandler::sendProtobufResponse(const QByteArray& message) {
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

bool AsyncMessageHandler::sendProtobufChunk(quint16 requestId,
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

    qDebug() << Q_FUNC_INFO << "Garmin: Chunk message size: " << msg.size() <<" bytes";

    // Rust: send_response(&message).await => enqueue
    return sendResponse(msg);
}

bool AsyncMessageHandler::handleProtobufChunkAck(quint16 requestId, quint32 dataOffset) {
    std::optional<PendingProtobufChunk> pendingChunk;

    {
        auto it = mPendingProtobufChunks.find(requestId);
        if (it != mPendingProtobufChunks.end()) {
            pendingChunk = it.value(); // cloned()
        }
    }

    if (!pendingChunk.has_value()) {
        emit logWarn(QStringLiteral("ℹ️ No pending chunk found for request ID %1").arg(requestId));
        return true;
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
            qDebug() << Q_FUNC_INFO << "Garmin: ERROR: Chunk boundary exceeds payload! offset" << nextOffset
                    << " size=" << chunkSize << " total=" << chunkInfo.completePayload.size();
            return true; // Rust returns Ok(())
        }

        const QByteArray nextChunk = chunkInfo.completePayload.mid(nextOffset, chunkSize);

        return sendProtobufChunk(requestId,
                                 quint32(nextOffset),
                                 nextChunk,
                                 chunkInfo.totalLength);
    }

    // Done: remove pending
    {
        mPendingProtobufChunks.remove(requestId);
    }
    emit logInfo(QStringLiteral("✅ All chunks sent for request ID %1").arg(requestId));
    emit logDebug(QStringLiteral("🎉 Transfer complete! Removed from pending chunks."));

    return true;
}

void AsyncMessageHandler::parse(const UnknownMessage& msg) {
    qDebug() << Q_FUNC_INFO;
    AsyncMessageHandler handler;
    if(!mCommunicator) {
        qDebug() << Q_FUNC_INFO << "Garmin: No communicator found.";
        return;
    }
    // ---------------------------------------------------------------------
    // Handle MusicControlCapabilities (0x13B2 = 5042)
    // ---------------------------------------------------------------------
    if (msg.messageId== 5042) {
        //Music control not fully implemented - sending ACK

        // Send simple ACK for MusicControlCapabilities
        QByteArray response_payload;
        response_payload.append(char(0xb2));
        response_payload.append(char(0x13)); // Original message ID: MusicControlCapabilities (5042)
        response_payload.append(char(0x00)); // Status: ACK
        QByteArray gfdi = wrapInGfdiEnvelope(0x1388, response_payload);
        mCommunicator->sendMessage("MusicAck",gfdi);
    }
    // ---------------------------------------------------------------------
    // Handle Response/Status messages (0x1388 = 5000)
    // ---------------------------------------------------------------------
    else if (msg.messageId == 0x1388) {
        qDebug() << Q_FUNC_INFO << "Garmin: RESPONSE/STATUS MESSAGE (0x1388) Full data ({}" << msg.data.size() << " bytes): " << msg.data.data();

        // Parse Response message
        if (msg.data.size() >= 4) {
            quint16 original_msg_id = u16le(msg.data.constData(),0);
            quint8 status = u8le (msg.data.constData(),2);

            // Check if this is an ACK for a ProtobufResponse (0x13B4) - need to send next chunk
            if ((original_msg_id == 0x13B4) && (status == 0) && (msg.data.size() >= 11)) {
                quint16 request_id = u16le(msg.data.constData(),3);
                quint32 data_offset = u32le(msg.data.constData(),5);

                qDebug() << Q_FUNC_INFO << "ACK for ProtobufResponse chunk,  Original msg ID: " << original_msg_id;
                if (status ==0) qDebug() << Q_FUNC_INFO << "Status: Ack";
                else qDebug() << Q_FUNC_INFO << "Status: Nack";

                if (!handleProtobufChunkAck(request_id, data_offset)) {
                        qDebug()<< Q_FUNC_INFO << "Garmin: Failed to handle protobuf chunk";
                }
            }

            // It's a Response to NotificationData (0x13AB)
            else if (original_msg_id == 0x13ab) {
                 const quint8 transferStatus = msg.data.size() > 3 ? u8le (msg.data.constData(),3) : 0;
                 qDebug() << Q_FUNC_INFO << "Garmin: Response to NotificationData (0x13AB)" << " Status:" << status << (status == 0 ? "(ACK)" : "(ERROR)");
                 qDebug() << Q_FUNC_INFO << "Garmin: Transfer Status:" << transferStatus << transferStatusName(transferStatus);

                 if (status == 0 && transferStatus == 0) {
                     qDebug() << Q_FUNC_INFO << "Garmin: NotificationData upload confirmed by watch!";
                     qDebug() << Q_FUNC_INFO << "Garmin: Watch has accepted the notification data";
                     qDebug() << Q_FUNC_INFO << "Garmin: Now sending final status ACK to complete handshake...";

                     QByteArray finalStatusPayload;
                     writeU16le(finalStatusPayload, 0x13AB); // NotificationData
                     finalStatusPayload.append(char(0x00));  // Status: ACK
                     finalStatusPayload.append(char(0x00));  // TransferStatus: OK

                     const QByteArray finalStatusMsg =
                         wrapInGfdiEnvelope(0x1388, finalStatusPayload);

                     qDebug() << Q_FUNC_INFO << "Garmin: Sending final status ACK (0x1388) to watch";

                     CommunicatorV2* comm= mCommunicator;

                     if (comm) {
                         bool sendRes = comm->sendMessage(
                             QStringLiteral("upload_complete"),
                             finalStatusMsg);

                         if (sendRes) {
                             qDebug() << Q_FUNC_INFO <<"Garmin: Final status ACK sent successfully!";
                             qDebug() << Q_FUNC_INFO << "Garmin: Upload handshake complete - notification should appear on watch!";
                         } else {
                             qDebug() << Q_FUNC_INFO << "Garmin: Failed to send final status ACK";
                         }
                     } else {
                         qDebug() << Q_FUNC_INFO << "Garmin: Communicator not available";
                     }
                 } else if (transferStatus == 3) {
                     qDebug() << Q_FUNC_INFO << "Garmin: CRC mismatch - watch rejected the data!";
                     qDebug() << Q_FUNC_INFO << "Garmin: The watch calculated a different CRC";
                     qDebug() << Q_FUNC_INFO << "Garmin: This indicates data corruption or incorrect CRC calculation";
                 } else if (transferStatus == 4) {
                     qDebug() << Q_FUNC_INFO << "Garmin: Offset mismatch - chunking issue!";
                     qDebug() << Q_FUNC_INFO << "Garmin: The watch expected data at a different offset";
                     qDebug() << Q_FUNC_INFO << "Garmin: This indicates chunking protocol mismatch";
                 } else if (status != 0) {
                     qDebug() << Q_FUNC_INFO << "Garmin: Watch returned error status:" << status;
                 } else if (transferStatus == 1) {
                     qDebug() << Q_FUNC_INFO << "Garmin: Watch requesting RESEND";
                     qDebug() << Q_FUNC_INFO << "Garmin: We may need to resend the same data";
                 } else if (transferStatus == 2) {
                     qDebug() << Q_FUNC_INFO << "Garmin: Watch sent ABORT";
                     qDebug() << Q_FUNC_INFO << "Garmin: Upload was cancelled by watch";
                 }
             } else {
                 qDebug() << Q_FUNC_INFO << "Garmin: Response to message: " << original_msg_id<< responseMessageName(original_msg_id) << " Status:" << status << (status == 0 ? "(ACK)" : "(ERROR)");;
             }
        }
        else {
         qDebug() << Q_FUNC_INFO <<"Response data too short (" << msg.data.size() << "bytes)";
        }
    }
    // ---------------------------------------------------------------------
    // Handle ProtobufRequest (0x13B3 = 5043)
    // ---------------------------------------------------------------------

    else if (msg.messageId == 0x13B3) {
         qDebug() << Q_FUNC_INFO << "Garmin: This is a ProtobufRequest - parsing protobuf payload";

         if (msg.data.size() >= 18) {
             const quint16 requestId = u16le(msg.data, 0);
             const quint32 dataOffset = u32le(msg.data, 2);
             const quint32 totalProtobufLength = u32le(msg.data, 6);
             const quint32 protobufDataLength = u32le(msg.data, 10);

             qDebug() << Q_FUNC_INFO << "Garmin: Request ID:" << requestId;
             qDebug() << Q_FUNC_INFO << "Garmin: Data Offset:" << dataOffset;
             qDebug() << Q_FUNC_INFO << "Garmin: Total Protobuf Length:" << totalProtobufLength;
             qDebug() << Q_FUNC_INFO << "Garmin: Protobuf Data Length:" << protobufDataLength;

             const int protobufStart = 14;
             const int protobufEnd = protobufStart + static_cast<int>(protobufDataLength);

             if (msg.data.size() >= protobufEnd) {
                 const QByteArray protobufPayload = msg.data.mid(protobufStart, protobufDataLength);


                 const bool isComplete =
                     (dataOffset == 0 && totalProtobufLength == protobufDataLength);

                 if (isComplete) {
                     qDebug() << Q_FUNC_INFO << "Garmin: Complete protobuf message - attempting to parse";

                     if (!protobufPayload.isEmpty()) {
                         const quint8 firstTag = static_cast<quint8>(protobufPayload[0]);
                         const quint8 fieldNumber = firstTag >> 3;
                         const quint8 wireType = firstTag & 0x07;

                         qDebug() << Q_FUNC_INFO << "Garmin: First protobuf field:" << fieldNumber
                                 << "(wire type:" << wireType << ") Payload: " << protobufPayload.toHex();

                         // =============================================================
                         // Field 1 => CalendarService/CoreService
                         // =============================================================
                         /* Not implemented yet
                         if (fieldNumber == 1 && wireType == 2)
                            {
                             qInfo() << "   ℹ️  Detected CalendarService/CoreService request (field 1)";

                             auto calReqRes = parseCalendarRequest(protobufPayload);
                             if (calReqRes.isOk()) {
                                 const CalendarRequest calendarRequest = calReqRes.value();

                                 qInfo() << "   📅 Calendar Service Request detected!";
                                 qInfo() << "      Time range:" << calendarRequest.startDate
                                         << "to" << calendarRequest.endDate;
                                 qInfo() << "      Max events:" << calendarRequest.maxEvents;
                                 qInfo() << "      Include all-day:" << calendarRequest.includeAllDay;

                                 qInfo() << "      📤 Sending ACK for calendar request...";

                                 QByteArray ackPayload;
                                 ackPayload.append(char(0xB3));
                                 ackPayload.append(char(0x13));
                                 ackPayload.append(char(0x00)); // ACK
                                 appendLe16(ackPayload, requestId);
                                 appendLe32(ackPayload, dataOffset);
                                 ackPayload.append(char(0x00)); // ProtobufChunkStatus: KEPT
                                 ackPayload.append(char(0x00)); // ProtobufStatusCode: NO_ERROR

                                 const QByteArray ackResponse =
                                     wrapInGfdiEnvelope(5000, ackPayload);

                                 auto ackRes = sendResponse(ackResponse);
                                 if (ackRes.isErr()) {
                                     qCritical().noquote()
                                         << QStringLiteral("      ❌ Failed to send ACK: %1")
                                                .arg(ackRes.error());
                                 } else {
                                     qInfo() << "      ✅ Calendar request ACK sent";
                                 }

                                 std::shared_ptr<CalendarManager> calendarManager;
                                 {
                                     QMutexLocker locker(&m_calendarManagerMutex);
                                     if (m_calendarManager) {
                                         calendarManager = m_calendarManager;
                                     }
                                 }

                                 if (!calendarManager) {
                                     qWarning() << "      ⚠️  Calendar manager not initialized - sending empty response";
                                 }

                                 qInfo() << "      🔄 Fetching calendar events...";

                                 // Assumed existing Qt-side helper equivalent to the Rust timeout wrapper
                                 auto fetchResult = handleCalendarRequestWithTimeout(
                                     calendarRequest,
                                     calendarManager.get(),
                                     15);

                                 if (fetchResult.isOk()) {
                                     const auto events = fetchResult.value();

                                     qInfo() << "      ✅ Fetched" << events.size() << "calendar events";

                                     const QByteArray responseData =
                                         encodeCalendarResponse(
                                             events,
                                             CalendarResponseStatus::Ok,
                                             requestId,
                                             calendarRequest.useCoreServiceEnvelope);

                                     const QByteArray response =
                                         wrapInGfdiEnvelope(0x13B4, responseData);

                                     qInfo() << "      📤 Sending calendar response ("
                                             << responseData.size() << "bytes)";

                                     std::shared_ptr<Communicator> comm;
                                     {
                                         QMutexLocker locker(&m_communicatorMutex);
                                         if (m_communicator) {
                                             comm = m_communicator;
                                         }
                                     }

                                     if (comm) {
                                         auto sendRes = comm->sendMessage(
                                             QStringLiteral("calendar_response"),
                                             response);
                                         if (sendRes.isOk()) {
                                             qInfo() << "      ✅ Calendar response sent successfully!";
                                         } else {
                                             qCritical().noquote()
                                                 << QStringLiteral("      ❌ Failed to send calendar response: %1")
                                                        .arg(sendRes.error());
                                         }
                                     } else {
                                         qCritical() << "      ❌ Failed to send calendar response: communicator not available";
                                     }
                                 } else if (fetchResult.isTimeout()) {
                                     qCritical() << "      ❌ Calendar request timeout";

                                     const QByteArray responseData =
                                         encodeCalendarResponse(
                                             {},
                                             CalendarResponseStatus::InvalidDateRange,
                                             requestId,
                                             calendarRequest.useCoreServiceEnvelope);

                                     const QByteArray response =
                                         wrapInGfdiEnvelope(0x13B4, responseData);

                                     std::shared_ptr<Communicator> comm;
                                     {
                                         QMutexLocker locker(&m_communicatorMutex);
                                         if (m_communicator) {
                                             comm = m_communicator;
                                         }
                                     }

                                     if (comm) {
                                         auto sendRes = comm->sendMessage(
                                             QStringLiteral("calendar_error_response"),
                                             response);
                                         if (sendRes.isOk()) {
                                             qInfo() << "      ✅ Calendar error response sent";
                                         } else {
                                             qCritical().noquote()
                                                 << QStringLiteral("      ❌ Failed to send error response: %1")
                                                        .arg(sendRes.error());
                                         }
                                     } else {
                                         qCritical() << "      ❌ Failed to send error response: communicator not available";
                                     }
                                 } else {
                                     qCritical().noquote()
                                         << QStringLiteral("      ❌ Calendar fetch error: %1")
                                                .arg(fetchResult.error());
                                 }
                             } else {
                                 qInfo().noquote()
                                     << QStringLiteral("      ⚠️  Not a calendar request or parse error: %1")
                                            .arg(calReqRes.error());
                                 qInfo() << "      💡 Sending generic ACK";

                                 QByteArray responsePayload;
                                 responsePayload.append(char(0xB3));
                                 responsePayload.append(char(0x13));
                                 responsePayload.append(char(0x00)); // ACK
                                 appendLe16(responsePayload, requestId);
                                 appendLe32(responsePayload, dataOffset);
                                 responsePayload.append(char(0x00)); // KEPT
                                 responsePayload.append(char(0x00)); // NO_ERROR

                                 const QByteArray response =
                                     wrapInGfdiEnvelope(5000, responsePayload);

                                 auto queueRes = sendResponse(response);
                                 if (queueRes.isErr()) {
                                     qCritical().noquote()
                                         << QStringLiteral("   ❌ Failed to queue ACK: %1")
                                                .arg(queueRes.error());
                                 } else {
                                     qInfo() << "   ✅ Generic ACK queued";
                                 }
                             }
                         }
                         // =============================================================
                         // Field 2 => HttpService
                         // =============================================================
                         else if (fieldNumber == 2 && wireType == 2) {
                             qInfo() << "   ℹ️  Detected HttpService request (field 2)";

                             auto httpReqRes = HttpRequest::parse(protobufPayload);
                             if (httpReqRes.isOk()) {
                                 const HttpRequest httpRequest = httpReqRes.value();
                                 Q_UNUSED(httpRequest.requestField);

                                 qInfo().noquote()
                                     << QStringLiteral("   🌐 HTTP %1 %2")
                                            .arg(httpRequest.method.asString(),
                                                 httpRequest.path);

                                 auto httpResponse =
                                     handleHttpRequestWithWeather(
                                         httpRequest,
                                         &m_weatherProvider);

                                 qInfo() << "   ✅ HTTP Response:" << httpResponse.status;

                                 auto encRes = httpResponse.encodeProtobufResponse(
                                     requestId,
                                     httpRequest,
                                     &m_dataTransferHandler);

                                 if (encRes.isOk()) {
                                     qInfo() << "   ✅ Sending ProtobufResponse with HTTP data";
                                     auto sendRes = sendProtobufResponse(encRes.value());
                                     if (sendRes.isOk()) {
                                         qInfo() << "   ✅ HTTP response sent successfully!";
                                     } else {
                                         qCritical().noquote()
                                             << QStringLiteral("   ❌ Failed to send HTTP response: %1")
                                                    .arg(sendRes.error());
                                     }
                                 } else {
                                     qCritical().noquote()
                                         << QStringLiteral("   ❌ Failed to encode HTTP response: %1")
                                                .arg(encRes.error());
                                 }
                             } else {
                                 qCritical().noquote()
                                     << QStringLiteral("   ❌ Failed to parse HTTP request: %1")
                                            .arg(httpReqRes.error());

                                 QByteArray responsePayload;
                                 responsePayload.append(char(0xB3));
                                 responsePayload.append(char(0x13));
                                 responsePayload.append(char(0x01)); // NACK
                                 appendLe16(responsePayload, requestId);
                                 appendLe32(responsePayload, dataOffset);
                                 responsePayload.append(char(0x00)); // KEPT
                                 responsePayload.append(char(0x00)); // NO_ERROR

                                 const QByteArray response =
                                     wrapInGfdiEnvelope(5000, responsePayload);

                                 auto queueRes = sendResponse(response);
                                 if (queueRes.isErr()) {
                                     qCritical().noquote()
                                         << QStringLiteral("   ❌ Failed to queue error ACK: %1")
                                                .arg(queueRes.error());
                                 } else {
                                     qInfo() << "   ✅ Error ACK queued";
                                 }
                             }
                         }
                         */
                         // =============================================================
                         // Field 8 => DeviceStatusService
                         // =============================================================
                         if (fieldNumber == 8 && wireType == 2) {
                             qDebug() << Q_FUNC_INFO << "Garmin: Detected DeviceStatusService request";
                             //Nestedservice so need to find inner service first
                             const quint16 requestId = u16le(protobufPayload, 0);
                             const quint32 dataOffset = u32le(protobufPayload, 2);
                             const quint32 totalProtobufLength = u32le(protobufPayload, 6);
                             const quint32 protobufDataLength = u32le(protobufPayload, 10);

                             qDebug() << Q_FUNC_INFO << "Garmin: Inner Request ID:" << requestId;
                             qDebug() << Q_FUNC_INFO << "Garmin: Inner Data Offset:" << dataOffset;
                             qDebug() << Q_FUNC_INFO << "Garmin: Inner Total Protobuf Length:" << totalProtobufLength;
                             qDebug() << Q_FUNC_INFO << "Garmin: Inner Protobuf Data Length:" << protobufDataLength;
                             const QByteArray protobufPayload2 =protobufPayload.mid(protobufStart, protobufDataLength);

                             qDebug() << Q_FUNC_INFO << "Garmin: Inner payload:" << protobufPayload2.toHex();



                             QByteArray responsePayload;
                             responsePayload.append(char(0xB3));
                             responsePayload.append(char(0x13));
                             responsePayload.append(char(0x00)); // ACK
                             writeU16le(responsePayload, requestId);
                             writeU32le(responsePayload, dataOffset);
                             responsePayload.append(char(0x00)); // KEPT
                             responsePayload.append(char(0x00)); // NO_ERROR

                             const QByteArray response =
                                 wrapInGfdiEnvelope(5000, responsePayload);

                             qDebug() << Q_FUNC_INFO << "Garmin: Sending ProtobufStatus ACK for DeviceStatusService";

                             bool queueRes = sendResponse(response);
                             if (!queueRes) {
                                 qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue ACK:";
                             } else {
                                 qDebug() << Q_FUNC_INFO << "Garmin: ProtobufStatus ACK queued - watch should continue!";
                             }
                         }
                         // =============================================================
                         // Field 27 => Authentication Request
                         // =============================================================
                         else if (fieldNumber == 27 && wireType == 2) {
                             qDebug() << Q_FUNC_INFO << "Garmin: Detected Authentication request";
                             qDebug() << Q_FUNC_INFO << "Garmin: Sending back 0 as we don't know what else to do";

                             QByteArray responsePayload;
                             writeU16le(responsePayload, requestId);
                             responsePayload.append(char(0));
                             writeU32le(responsePayload,0);

                             const QByteArray response =
                                 wrapInGfdiEnvelope(5101, responsePayload);

                             bool queueRes = sendResponse(response);
                             if (!queueRes) {
                                 qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue ACK:";
                             } else {
                                 qDebug() << Q_FUNC_INFO << "Garmin: Authentication Response sent";
                             }
                         }
                         // =============================================================
                         // Field 7 => DataTransfer chunk fetch request
                         // =============================================================
/* not implemented yet
                         else if (fieldNumber == 7 && wireType == 2) {
                             qInfo() << "   ℹ️  Detected DataTransfer chunk fetch request (field 7)";

                             int pos = 1; // skip field tag
                             quint32 length = 0;
                             int shift = 0;

                             while (pos < protobufPayload.size()) {
                                 const quint8 byte = static_cast<quint8>(protobufPayload[pos]);
                                 ++pos;
                                 length |= (quint32(byte & 0x7F) << shift);
                                 if ((byte & 0x80) == 0) {
                                     break;
                                 }
                                 shift += 7;
                             }

                             if (pos + static_cast<int>(length) <= protobufPayload.size()) {
                                 const QByteArray field7Data = protobufPayload.mid(pos, length);

                                 std::optional<quint32> transferId;
                                 std::optional<quint32> offset;
                                 std::optional<quint32> chunkLength;

                                 int parsePos = 0;
                                 while (parsePos < field7Data.size()) {
                                     const quint8 tag = static_cast<quint8>(field7Data[parsePos]);
                                     const quint8 fieldNum = tag >> 3;
                                     ++parsePos;

                                     quint32 value = 0;
                                     int valueShift = 0;
                                     while (parsePos < field7Data.size()) {
                                         const quint8 byte =
                                             static_cast<quint8>(field7Data[parsePos]);
                                         ++parsePos;
                                         value |= (quint32(byte & 0x7F) << valueShift);
                                         if ((byte & 0x80) == 0) {
                                             break;
                                         }
                                         valueShift += 7;
                                     }

                                     switch (fieldNum) {
                                     case 1: transferId = value; break;
                                     case 2: offset = value; break;
                                     case 3: chunkLength = value; break;
                                     default: break;
                                     }
                                 }

                                 if (transferId && offset && chunkLength) {
                                     qInfo() << "   📦 DataTransfer chunk request:";
                                     qInfo() << "      Transfer ID:" << *transferId;
                                     qInfo() << "      Offset:" << *offset;
                                     qInfo() << "      Length:" << *chunkLength;

                                     auto chunkOpt = m_dataTransferHandler.getChunk(
                                         *transferId,
                                         static_cast<int>(*offset),
                                         static_cast<int>(*chunkLength));

                                     if (chunkOpt.has_value()) {
                                         const QByteArray chunk = chunkOpt.value();

                                         qInfo() << "   ✅ Serving chunk:" << chunk.size() << "bytes";

                                         QByteArray downloadResponse;

                                         // Field 1: status = 1 (SUCCESS)
                                         downloadResponse.append(char((1 << 3) | 0));
                                         downloadResponse.append(char(1));

                                         // Field 2: id
                                         downloadResponse.append(char((2 << 3) | 0));
                                         appendVarint(downloadResponse, *transferId);

                                         // Field 3: offset
                                         downloadResponse.append(char((3 << 3) | 0));
                                         appendVarint(downloadResponse, *offset);

                                         // Field 4: payload
                                         downloadResponse.append(char((4 << 3) | 2));
                                         appendVarint(downloadResponse, static_cast<quint32>(chunk.size()));
                                         downloadResponse.append(chunk);

                                         // Wrap in DataTransferService field 2
                                         QByteArray dataTransferService;
                                         dataTransferService.append(char((2 << 3) | 2));
                                         appendVarint(dataTransferService,
                                                      static_cast<quint32>(downloadResponse.size()));
                                         dataTransferService.append(downloadResponse);

                                         // Wrap in Smart proto field 7
                                         QByteArray smartProto;
                                         smartProto.append(char((7 << 3) | 2));
                                         appendVarint(smartProto,
                                                      static_cast<quint32>(dataTransferService.size()));
                                         smartProto.append(dataTransferService);

                                         // Build ProtobufResponse packet manually
                                         QByteArray packet;
                                         packet.append(char(0x00));
                                         packet.append(char(0x00)); // size placeholder
                                         appendLe16(packet, 5044);  // PROTOBUF_RESPONSE
                                         appendLe16(packet, requestId);
                                         appendLe32(packet, 0u); // dataOffset
                                         appendLe32(packet, static_cast<quint32>(smartProto.size())); // total len
                                         appendLe32(packet, static_cast<quint32>(smartProto.size())); // data len
                                         packet.append(smartProto);

                                         const quint16 packetSize =
                                             static_cast<quint16>(packet.size() + 2);
                                         packet[0] = static_cast<char>(packetSize & 0xFF);
                                         packet[1] = static_cast<char>((packetSize >> 8) & 0xFF);

                                         const quint16 checksum = computeCrc16(packet);
                                         appendLe16(packet, checksum);

                                         qInfo() << "   📤 Sending DataTransfer chunk response ("
                                                 << packet.size() << "bytes)";

                                         std::shared_ptr<Communicator> comm;
                                         {
                                             QMutexLocker locker(&m_communicatorMutex);
                                             if (m_communicator) {
                                                 comm = m_communicator;
                                             }
                                         }

                                         if (comm) {
                                             auto sendRes = comm->sendMessage(
                                                 QStringLiteral("data_transfer_chunk"),
                                                 packet);
                                             if (sendRes.isOk()) {
                                                 qInfo() << "   ✅ DataTransfer chunk sent";
                                             } else {
                                                 qCritical().noquote()
                                                     << QStringLiteral("   ❌ Failed to send chunk: %1")
                                                            .arg(sendRes.error());
                                             }
                                         } else {
                                             qCritical() << "   ❌ Failed to send chunk: communicator not available";
                                         }
                                     } else {
                                         qWarning().noquote()
                                             << QStringLiteral("   ❌ Transfer ID %1 not found or invalid offset")
                                                    .arg(*transferId);

                                         QByteArray responsePayload;
                                         responsePayload.append(char(0xB3));
                                         responsePayload.append(char(0x13));
                                         responsePayload.append(char(0x01)); // NACK
                                         appendLe16(responsePayload, requestId);
                                         appendLe32(responsePayload, dataOffset);
                                         responsePayload.append(char(0x00));
                                         responsePayload.append(char(0x01)); // error code

                                         const QByteArray response =
                                             wrapInGfdiEnvelope(5000, responsePayload);

                                         std::shared_ptr<Communicator> comm;
                                         {
                                             QMutexLocker locker(&m_communicatorMutex);
                                             if (m_communicator) {
                                                 comm = m_communicator;
                                             }
                                         }

                                         if (comm) {
                                             auto sendRes = comm->sendMessage(
                                                 QStringLiteral("data_transfer_error"),
                                                 response);
                                             if (sendRes.isOk()) {
                                                 qInfo() << "   ✅ DataTransfer error response sent";
                                             } else {
                                                 qCritical().noquote()
                                                     << QStringLiteral("   ❌ Failed to send error: %1")
                                                            .arg(sendRes.error());
                                             }
                                         } else {
                                             qCritical() << "   ❌ Failed to send error: communicator not available";
                                         }
                                     }
                                 } else {
                                     qWarning() << "   ❌ Failed to parse DataTransfer request fields";
                                     qWarning() << "   💡 Not a DataTransfer request - sending generic ACK";

                                     QByteArray responsePayload;
                                     responsePayload.append(char(0xB3));
                                     responsePayload.append(char(0x13));
                                     responsePayload.append(char(0x00)); // ACK
                                     appendLe16(responsePayload, requestId);
                                     appendLe32(responsePayload, dataOffset);
                                     responsePayload.append(char(0x00));
                                     responsePayload.append(char(0x00));

                                     const QByteArray response =
                                         wrapInGfdiEnvelope(5000, responsePayload);

                                     auto queueRes = sendResponse(response);
                                     if (queueRes.isErr()) {
                                         qCritical().noquote()
                                             << QStringLiteral("   ❌ Failed to queue ACK: %1")
                                                    .arg(queueRes.error());
                                     } else {
                                         qInfo() << "   ✅ Generic ACK queued";
                                     }
                                 }
                             } else {
                                 qWarning() << "   ❌ Invalid field 7 protobuf length";
                             }
                         }
                         // =============================================================
                         // Unknown field => try HTTP parse
                         // =============================================================
                         else {
                             qWarning() << "   ⚠️  Unknown protobuf field" << fieldNumber
                                        << "- attempting HTTP parse";

                             auto httpReqRes = HttpRequest::parse(protobufPayload);
                             if (httpReqRes.isOk()) {
                                 const HttpRequest httpRequest = httpReqRes.value();
                                 Q_UNUSED(httpRequest.requestField);

                                 qInfo().noquote()
                                     << QStringLiteral("   🌐 HTTP %1 %2 (from field %3)")
                                            .arg(httpRequest.method.asString(),
                                                 httpRequest.path)
                                            .arg(fieldNumber);

                                 auto httpResponse =
                                     handleHttpRequestWithWeather(
                                         httpRequest,
                                         &m_weatherProvider);

                                 qInfo() << "   ✅ HTTP Response:" << httpResponse.status;

                                 auto encRes = httpResponse.encodeProtobufResponse(
                                     requestId,
                                     httpRequest,
                                     &m_dataTransferHandler);

                                 if (encRes.isOk()) {
                                     qInfo() << "   ✅ Sending ProtobufResponse with HTTP data";

                                     auto sendRes = sendProtobufResponse(encRes.value());
                                     if (sendRes.isErr()) {
                                         qCritical().noquote()
                                             << QStringLiteral("   ❌ Failed to send HTTP response: %1")
                                                    .arg(sendRes.error());
                                     } else {
                                         qInfo() << "   ✅ HTTP response queued";
                                     }
                                 } else {
                                     qCritical().noquote()
                                         << QStringLiteral("   ❌ Failed to encode HTTP response: %1")
                                                .arg(encRes.error());
                                 }
                             } else {
                                 qInfo() << "   💡 Not an HTTP request - sending generic ACK";

                                 QByteArray responsePayload;
                                 responsePayload.append(char(0xB3));
                                 responsePayload.append(char(0x13));
                                 responsePayload.append(char(0x00)); // ACK
                                 appendLe16(responsePayload, requestId);
                                 appendLe32(responsePayload, dataOffset);
                                 responsePayload.append(char(0x00)); // KEPT
                                 responsePayload.append(char(0x00)); // NO_ERROR

                                 const QByteArray response =
                                     wrapInGfdiEnvelope(5000, responsePayload);

                                 auto queueRes = sendResponse(response);
                                 if (queueRes.isErr()) {
                                     qCritical().noquote()
                                         << QStringLiteral("   ❌ Failed to queue ACK: %1")
                                                .arg(queueRes.error());
                                 } else {
                                     qInfo() << "   ✅ Generic ACK queued";
                                 }
                             }
                         }
*/
                        else {
                             QByteArray responsePayload;
                             responsePayload.append(char(0xB3));
                             responsePayload.append(char(0x13));
                             responsePayload.append(char(0x00)); // ACK
                             writeU16le(responsePayload, requestId);
                             writeU32le(responsePayload, dataOffset);
                             responsePayload.append(char(0x00)); // KEPT
                             responsePayload.append(char(0x00)); // NO_ERROR

                             const QByteArray response =
                                 wrapInGfdiEnvelope(5000, responsePayload);

                             qDebug() << Q_FUNC_INFO << "Garmin: Unknown Protobuf detected, sending generic ACK";

                             bool queueRes = sendResponse(response);
                             if (!queueRes) {
                                 qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue ACK:";
                             } else {
                                 qDebug() << Q_FUNC_INFO << "Garmin: ProtobufStatus ACK queued - watch should continue!";
                             }
                         }
                     }
                 } else {
                     qDebug() << Q_FUNC_INFO << "Garmin: Chunked protobuf message detected (not fully implemented)";
                     qDebug() << Q_FUNC_INFO << "Garmin: Data offset:" << dataOffset
                                << ", Total:" << totalProtobufLength
                                << ", Current:" << protobufDataLength;

                     QByteArray responsePayload;
                     responsePayload.append(char(0xB3));
                     responsePayload.append(char(0x13));
                     responsePayload.append(char(0x00)); // ACK
                     writeU16le(responsePayload, requestId);
                     writeU32le(responsePayload, dataOffset);
                     responsePayload.append(char(0x00)); // KEPT
                     responsePayload.append(char(0x00)); // NO_ERROR

                     const QByteArray response =
                         wrapInGfdiEnvelope(5000, responsePayload);

                     bool queueRes = sendResponse(response);
                     if (!queueRes) {
                         qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue chunk ACK";
                     } else {
                         qDebug() << Q_FUNC_INFO << "Garmin:  Chunk ACK queued";
                     }
                 }
             } else {
                 qDebug() << Q_FUNC_INFO << "Garmin: ProtobufRequest data too short for payload (expected" << protobufEnd <<" bytes, got" << msg.data.size() <<")";
             }
         } else {
             qDebug() << Q_FUNC_INFO << "Garmin:  ProtobufRequest data too short to parse full header (got" << msg.data.size() <<" bytes, need 18)";
         }
     }
}

