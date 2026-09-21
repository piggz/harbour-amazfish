#include "garminnotificationcontrolmessage.h"


const quint8 NOTIF_CMD_GET_NOTIFICATION_ATTRIBUTES = 0;

const quint8 NOTIF_ATTR_APP_IDENTIFIER = 0;
const quint8 NOTIF_ATTR_TITLE = 1;
const quint8 NOTIF_ATTR_SUBTITLE = 2;
const quint8 NOTIF_ATTR_MESSAGE = 3;
const quint8 NOTIF_ATTR_MESSAGE_SIZE = 4;
const quint8 NOTIF_ATTR_DATE = 5;
const quint8 NOTIF_ATTR_ACTIONS = 127;
const quint8 NOTIF_ATTR_ATTACHMENTS = 128;

bool notificationAttributeHasLengthParam(quint8 code)
{
    return code == NOTIF_ATTR_TITLE || code == NOTIF_ATTR_SUBTITLE || code == NOTIF_ATTR_MESSAGE;
}

bool notificationAttributeHasAdditionalParams(quint8 code)
{
    return code == NOTIF_ATTR_ACTIONS;
}

// std::nullopt means "we don't supply this attribute" (the TLV is omitted
// entirely, matching how Gadgetbridge just skips ATTACHMENTS/etc. when the
// notification doesn't have one).
std::optional<QByteArray> notificationAttributeValue(quint8 code, int maxLength, const QString &appIdentifier, const QString &title, const QString &message)
{
    QString text;
    switch (code) {
    case NOTIF_ATTR_APP_IDENTIFIER:
        text = appIdentifier;
        break;
    case NOTIF_ATTR_TITLE:
        text = title;
        break;
    case NOTIF_ATTR_SUBTITLE:
        text = QString();
        break;
    case NOTIF_ATTR_MESSAGE:
        text = message;
        break;
    case NOTIF_ATTR_MESSAGE_SIZE:
        text = QString::number(message.length());
        break;
    case NOTIF_ATTR_DATE:
        text = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd'T'HHmmss"));
        break;
    case NOTIF_ATTR_ACTIONS:
        // No attached actions: 4 zero bytes is the "no actions" marker.
        return QByteArray(4, char(0));
    case NOTIF_ATTR_ATTACHMENTS:
        return std::nullopt; // we never advertise a picture, so never claim one
    default:
        // Never silently omit an attribute the watch explicitly asked for -
        // it treats an incomplete reply as unsatisfactory and re-requests
        // forever. Reply with an empty value instead when we have nothing
        // real to say, matching Gadgetbridge's own attribute encoder (its
        // switch has no case for every possible code either, but its
        // default still always encodes something).
        return QByteArray();
    }

    if (maxLength > 0)
        text = text.left(maxLength);
    return text.toUtf8();
}



void GarminNotificationControlMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification control";
    if (data.isEmpty()) {
        return;
    }

    // First ack the ControlMessge
    QByteArray ack;
    writeU16le(ack, quint16(MessageId::NotificationControl));
    ack.append(char(Status::Ack));
    ack.append(char(0)); // NotificationChunkStatus::OK
    ack.append(char(0)); // NotificationStatusCode::NO_ERROR
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION_CONTROL_ACK",wrapInGfdiEnvelope(MessageId::Response,ack));


    NotificationControlMessage msg;
    msg.command = quint8(data[0]);
    msg.notificationId = i32le(data, 1);

    if (msg.command != 0 || data.size() < 5) {
        qDebug() << Q_FUNC_INFO << QStringLiteral("GFDI: Unsupported notification control command %1").arg(msg.command);
        return;
    }
//TEST CODE
    /*
    const auto it = m_pendingNotifications.constFind(int(notificationId));
    if (it == m_pendingNotifications.constEnd()) {
        logEvent(QStringLiteral("GFDI: Watch requested unknown notification %1").arg(notificationId));
        return;
    }
    */
    const QByteArray attributeRequest = data.mid(5);

    QByteArray attributes;
    attributes.append(char(msg.command));
    writeU32le(attributes, msg.notificationId);
    QByteArray messageSizeAttribute;

    int pos = 0;
    while (pos < attributeRequest.size()) {
        const quint8 code = quint8(attributeRequest.at(pos++));
        int maxLength = 0;
        if (notificationAttributeHasLengthParam(code)) {
            if (pos + 2 > attributeRequest.size())
                break;
            maxLength = u16le(attributeRequest, pos);
            pos += 2;
        } else if (notificationAttributeHasAdditionalParams(code)) {
            if (pos + 3 > attributeRequest.size())
                break;
            maxLength = u16le(attributeRequest, pos);
            pos += 3; // 2-byte param + 1 unknown byte we don't use
        }

        const std::optional<QByteArray> value = notificationAttributeValue(code, maxLength, QString("SMS"), QString("TestTitel"), QString("TestNachricht"));
        if (!value)
            continue;

        QByteArray tlv;
        tlv.append(char(code));
        writeU16le(tlv, value->size());
        tlv.append(*value);

        if (code == NOTIF_ATTR_MESSAGE_SIZE)
            messageSizeAttribute = tlv;
        else
            attributes.append(tlv);
    }
    attributes.append(messageSizeAttribute);


    QByteArray dataPayload;
    writeU16le(dataPayload, attributes.size()); // messageSize
    writeU16le(dataPayload, computeCrc16(attributes)); // crc of this (only) chunk
    writeU16le(dataPayload, 0); // dataOffset
    dataPayload.append(attributes);

    if (mCommunicator)  mCommunicator->sendMessage("NOTIFICATIO_DATA",wrapInGfdiEnvelope(MessageId::NotificationData,dataPayload));
// Test code ends - need to implement the real data in Notifcationhandler.
 }

