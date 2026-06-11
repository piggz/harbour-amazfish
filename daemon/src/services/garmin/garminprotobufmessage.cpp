#include "garminprotobufmessage.h"
#include "garmindevicestatusmessage.h"
#include "garmincalendarmessage.h"
#include "garminconfigurationmessage.h"


void GarminProtobufMessage::parse(const QByteArray& data)
{
    qDebug() << Q_FUNC_INFO << "Garmin: This is a ProtobufRequest - parsing protobuf payload";

    if (data.size() >= 18) {
        const quint16 requestId = u16le(data, 0);
        const quint32 dataOffset = u32le(data, 2);
        const quint32 totalProtobufLength = u32le(data, 6);
        const quint32 protobufDataLength = u32le(data, 10);

        qDebug() << Q_FUNC_INFO << "Garmin: Request ID:" << requestId;
        qDebug() << Q_FUNC_INFO << "Garmin: Data Offset:" << dataOffset;
        qDebug() << Q_FUNC_INFO << "Garmin: Total Protobuf Length:" << totalProtobufLength;
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf Data Length:" << protobufDataLength;

        const int protobufStart = 14;
        const int protobufEnd = protobufStart + static_cast<int>(protobufDataLength);

        if (data.size() >= protobufEnd) {
            const QByteArray protobufPayload = data.mid(protobufStart, protobufDataLength);


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
                    if (fieldNumber == 1 && wireType == 2)
                       {
                        qInfo() << "   ℹ️  Detected CalendarService/CoreService request (field 1)";
                        handleCalendarRequest(protobufPayload,requestId,dataOffset);

                    }
                    /*
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
                    // Field 27 => Authentication Request
                    // =============================================================
                    if (fieldNumber == 27 && wireType == 2) {
                        qDebug() << Q_FUNC_INFO << "Garmin: Detected Authentication request";
                        //handleAuthenticationRequest(requestId);
                        handleAuthenticationRequest(u16le(protobufPayload,0));

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
                        sendGenericAck(data);
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

                if (mCommunicator) {
                    bool queueRes = mCommunicator->sendMessage("SEND CHUNK ACK",response);
                    if (!queueRes) {
                        qDebug() << Q_FUNC_INFO << "Garmin: Failed to queue chunk ACK";
                    } else {
                        qDebug() << Q_FUNC_INFO << "Garmin:  Chunk ACK queued";
                    }
                }
            }
        } else {
            qDebug() << Q_FUNC_INFO << "Garmin: ProtobufRequest data too short for payload (expected" << protobufEnd <<" bytes, got" << data.size() <<")";
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Garmin:  ProtobufRequest data too short to parse full header (got" << data.size() <<" bytes, need 18)";
    }

}

void GarminProtobufMessage::parseResponse(const QByteArray &data) {
    if (data.size() >= 18) {
        const quint16 requestId = u16le(data, 0);
        const quint32 dataOffset = u32le(data, 2);
        const quint32 totalProtobufLength = u32le(data, 6);
        const quint32 protobufDataLength = u32le(data, 10);

        qDebug() << Q_FUNC_INFO << "Garmin: Request ID:" << requestId;
        qDebug() << Q_FUNC_INFO << "Garmin: Data Offset:" << dataOffset;
        qDebug() << Q_FUNC_INFO << "Garmin: Total Protobuf Length:" << totalProtobufLength;
        qDebug() << Q_FUNC_INFO << "Garmin: Protobuf Data Length:" << protobufDataLength;

        const int protobufStart = 14;
        const int protobufEnd = protobufStart + static_cast<int>(protobufDataLength);

        if (data.size() >= protobufEnd) {
            const QByteArray protobufPayload = data.mid(protobufStart, protobufDataLength);
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

                    if (fieldNumber==8 && wireType ==2)
                    { //get length of inner protobuf
                        quint8  innerLength=protobufPayload[1];
                        GarminDeviceStatusMessage* msg = new GarminDeviceStatusMessage(mCommunicator);
                        msg->parse(protobufPayload.mid(2,innerLength));
                  }
                }
            }
        }
    }
}

void GarminProtobufMessage::handleAuthenticationRequest(quint16 authFlags) {
    qDebug() << Q_FUNC_INFO << "Garmin: Sending back 0 as we don't know what else to do";

    QByteArray responsePayload;
    writeU16le(responsePayload,5101);
    responsePayload.append(char(0));
    writeU16le(responsePayload,authFlags);
    const QByteArray response =
        wrapInGfdiEnvelope(5000, responsePayload);

    if (mCommunicator) {
        bool queueRes = mCommunicator->sendMessage("AUTHENTICATION ACK",response);
        if (!queueRes) {
            qDebug() << Q_FUNC_INFO << "Garmin: Failed to send ACK:";
        } else {
            qDebug() << Q_FUNC_INFO << "Garmin: Authentication Response sent";
        }
    }
}

void GarminProtobufMessage::handleCalendarRequest(const QByteArray& data, quint16 requestID, quint32 dataOffset)
{
    GarminCalendarMessage* calendarMsg = new GarminCalendarMessage(mCommunicator);
    calendarMsg->parse(data, requestID, dataOffset);
}

void GarminProtobufMessage::sendGenericAck(const QByteArray& data) {
    const quint16 requestId = u16le(data, 0);
    const quint32 dataOffset = u32le(data, 2);

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

    if (mCommunicator){
        bool queueRes = mCommunicator->sendMessage("GENERIC ACK",response);
        if (!queueRes) {
            qDebug() << Q_FUNC_INFO << "Garmin: Failed to send ACK:";
        } else {
            qDebug() << Q_FUNC_INFO << "Garmin: Generc ACK sent - watch should continue!";
        }
    }
}

