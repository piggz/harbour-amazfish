#include "garmingfdistatusmessage.h"
#include "garmindevicestatusmessage.h"
#include "garminauthnegotiationstatusmessage.h"

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

void GarminGfdiStatusMessage::parse(const QByteArray& data)
{
    // This handles Status / Response MEssages from teh watch.
    // Not all below messages are expected, but for debugging we notify for all
    quint16 originalMessageType = u16le(data.constData(),0);
    QByteArray response_payload;
    QByteArray gfdi;

    switch (static_cast<MessageId> (originalMessageType)) {
    case MessageId::AuthNegotiation:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        handleAuthNegotiation(data);
        break;
    case MessageId::Configuration:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::CreateFile:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::CurrentTimeRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::DeviceInformation:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::DeviceSettings:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::DownloadRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::FileTransferData:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::Filter:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::FindMyPhoneCancel:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::FindMyPhoneRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::FitData:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::FitDefinition:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::MusicControl:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::MusicControlCapabilities:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        //Music control not fully implemented - sending ACK
        response_payload.clear();
        writeU16le(response_payload,static_cast<quint16>(MessageId::MusicControlCapabilities));
        response_payload.append(char(Status::Ack));
        gfdi = wrapInGfdiEnvelope(static_cast<quint16>(MessageId::Response), response_payload);
        if (mCommunicator) mCommunicator->sendMessage("MUSIC ACC",gfdi);
        break;
    case MessageId::MusicControlEntityUpdate:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::NotificationControl:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::NotificationData:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        handleNotificationData(data);
        break;
    case MessageId::NotificationSubscription:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::NotificationUpdate:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::ProtobufRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::ProtobufResponse:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        handleProtobufResponse(data);
        break;
    case MessageId::Response:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::SetFileFlag:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::SupportedFileTypesRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::Synchronization:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::SystemEvent:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::UploadRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    case MessageId::WeatherRequest:
        qDebug() << Q_FUNC_INFO << "Garmin: Detected " << messageIdToString(originalMessageType).value();
        break;
    }
}

void GarminGfdiStatusMessage::handleAuthNegotiation(const QByteArray &data) {
    GarminAuthNegotiationStatusMessage* msg = new GarminAuthNegotiationStatusMessage(mCommunicator);
    msg->parse(data);
}


void GarminGfdiStatusMessage::handleDeviceConfiguration(const QByteArray &data) {
    //GarminConfigurationMessage* msg = new GarminConfigurationMessage(mCommunicator);
    //msg->parse(data);
}

void GarminGfdiStatusMessage::handleNotificationData(const QByteArray &data) {
    //quint16 original_msg_id = u16le(data.constData(),0);
    quint8 status = u8le (data.constData(),2);
    const quint8 transferStatus = data.size() > 3 ? u8le (data.constData(),3) : 0;
    qDebug() << Q_FUNC_INFO << "Garmin: Response to NotificationData (0x13AB)" << " Status:" << status << (status == 0 ? "(ACK)" : "(ERROR)");
    qDebug() << Q_FUNC_INFO << "Garmin: Transfer Status:" << transferStatus << transferStatusName(transferStatus);

    if (status == 0 && transferStatus == 0) {
        qDebug() << Q_FUNC_INFO << "Garmin: NotificationData upload confirmed by watch!";
        qDebug() << Q_FUNC_INFO << "Garmin: Now sending final status ACK to complete handshake...";

        QByteArray finalStatusPayload;
        writeU16le(finalStatusPayload, 0x13AB); // NotificationData
        finalStatusPayload.append(char(Status::Ack));  // Status: ACK
        finalStatusPayload.append(char(0x00));  // TransferStatus: OK

        const QByteArray finalStatusMsg =
            wrapInGfdiEnvelope(5000, finalStatusPayload);

        qDebug() << Q_FUNC_INFO << "Garmin: Sending final status ACK (5000) to watch";

        CommunicatorV2* comm= mCommunicator;

        if (comm) {
            bool sendRes = comm->sendMessage("UPLOAD COMPLETE",finalStatusMsg);
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
    } else if (transferStatus == 4) {
        qDebug() << Q_FUNC_INFO << "Garmin: Offset mismatch - chunking issue!";
    } else if (status != 0) {
        qDebug() << Q_FUNC_INFO << "Garmin: Watch returned error status:" << status;
    } else if (transferStatus == 1) {
        qDebug() << Q_FUNC_INFO << "Garmin: Watch requesting RESEND";
    } else if (transferStatus == 2) {
        qDebug() << Q_FUNC_INFO << "Garmin: Watch sent ABORT";
    }
}



void GarminGfdiStatusMessage::handleProtobufResponse(const QByteArray &data) {
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



