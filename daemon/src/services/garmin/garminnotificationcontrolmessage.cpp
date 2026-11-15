#include "garminnotificationcontrolmessage.h"

enum class NotificationCommand : quint8 {
    GET_NOTIFICATION_ATTRIBUTES =0,
    GET_APP_ATTRIBUTES = 1,
    PERFORM_LEGACY_NOTIFICATION_ACTION = 2,
    PERFORM_NOTIFICATION_ACTION = 128
};

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


    // GET_APP_ATTRIBUTES carries an app identifier instead of a notification id. The watch asks
    // for the app's name after showing a notification; without a reply it stops fetching new
    // notifications until it reconnects.
    if (quint8(data[0]) == quint8(NotificationCommand::GET_APP_ATTRIBUTES)) {
        sendAppAttributes(data);
        return;
    }

    NotificationControlMessage msg;
    msg.command = quint8(data[0]);
    msg.notificationId = i32le(data, 1);

    if (data.size() < 5)
        {
            qDebug() << Q_FUNC_INFO << "GFDI: Ntification control message to small";
            return;
        }
    switch (msg.command) {
        case (quint8) NotificationCommand::GET_NOTIFICATION_ATTRIBUTES:
            msg.data = data.mid(5);
            if (mCommunicator)mCommunicator->onNotificationDataRequested(msg);
            return;
        case (quint8) NotificationCommand::PERFORM_NOTIFICATION_ACTION:
            msg.data = data.mid(5);
            if (mCommunicator)mCommunicator->onNotificationPerformAction(msg);
            return;

        default:
            qDebug() << Q_FUNC_INFO << QStringLiteral("GFDI: Unsupported notification control command %1").arg(msg.command);

    }
}


// Reply to GET_APP_ATTRIBUTES: [command][app identifier, NUL-terminated][attribute codes...].
// Same reply as Gadgetbridge (NotificationsHandler.getAppAttributes), sent as NotificationData:
// [command][app identifier][0] and for each attribute [code][length u16][value]. The only
// attribute is APP_NAME (0); the identifier we send in notifications is already the app's name.
void GarminNotificationControlMessage::sendAppAttributes(const QByteArray& data)
{
    const int end = data.indexOf('\0', 1);
    if (end < 0) {
        qDebug() << Q_FUNC_INFO << "GFDI: app attributes request without app identifier";
        return;
    }
    const QByteArray appIdentifier = data.mid(1, end - 1);
    qDebug() << Q_FUNC_INFO << "Garmin: app attributes requested for" << QString::fromUtf8(appIdentifier);

    QByteArray attributes;
    attributes.append(char(NotificationCommand::GET_APP_ATTRIBUTES));
    attributes.append(appIdentifier);
    attributes.append(char(0));
    for (int pos = end + 1; pos < data.size(); ++pos) {
        const quint8 code = quint8(data.at(pos));
        const QByteArray value = code == 0 ? appIdentifier : QByteArray(); // 0 = APP_NAME
        attributes.append(char(code));
        writeU16le(attributes, value.size());
        attributes.append(value);
    }

    QByteArray dataPayload;
    writeU16le(dataPayload, attributes.size()); // messageSize
    writeU16le(dataPayload, computeCrc16(attributes)); // crc of this (only) chunk
    writeU16le(dataPayload, 0); // dataOffset
    dataPayload.append(attributes);
    if (mCommunicator)
        mCommunicator->sendMessage("NOTIFICATION_APP_ATTRIBUTES", wrapInGfdiEnvelope(MessageId::NotificationData, dataPayload));
}
