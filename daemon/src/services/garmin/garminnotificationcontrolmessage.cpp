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
        default:
            qDebug() << Q_FUNC_INFO << QStringLiteral("GFDI: Unsupported notification control command %1").arg(msg.command);

    }
}

