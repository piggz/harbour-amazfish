#include "garminnotificationcontrolmessage.h"

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

    if (msg.command != 0 || data.size() < 5) //NOTIF_CMD_GET_NOTIFICATION_ATTRIBUTES
    {
        qDebug() << Q_FUNC_INFO << QStringLiteral("GFDI: Unsupported notification control command %1").arg(msg.command);
        return;
    }
    msg.data = data.mid(5);
    if (mCommunicator)mCommunicator->onNotificationControlReceived(msg);

 }

