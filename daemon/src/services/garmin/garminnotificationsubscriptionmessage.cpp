#include "garminnotificationsubscriptionmessage.h"


void GarminNotificationSubscriptionMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification subscription";
    if (data.size() < 2) {
        return;
    }
    NotificationSubscriptionMessage msg;
    msg.enable = (quint8(data[0]) == 1);
    msg.unk = quint8(data[1]);

    QByteArray response = generateResponse(msg);
    response = wrapInGfdiEnvelope(5000,response);
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATIONSUBSCRIPTION RESPONSE",response);
}

QByteArray GarminNotificationSubscriptionMessage::generateResponse(NotificationSubscriptionMessage& msg) {

    // Generate a NotificationSubscription response
    //
    // This responds to the watch's notification subscription request (message ID 5036)
    // with an ACK and notification status information.
    //
    // # Arguments
    // * `incoming` - The incoming NotificationSubscriptionMessage
    // * `enabled` - Whether notifications are enabled (typically true)

    bool enabled =true;
    //todo: set notification handler enable in device
    //Todo: Check if notfications are enabled in device settings, setting to true for now.
    // could be in AmazfishConfig::deviceDisconnectNotification() ?

    QByteArray r;
    // Original message ID: NOTIFICATION_SUBSCRIPTION (5036)
    writeU16le(r, 5036);
    // Status: ACK
    r.append(char(quint8(Status::Ack)));
    // Notification Status (0 = ENABLED, 1 = DISABLED)
    r.append(char(enabled ? 0 : 1));
    // Enable flag (matches incoming request)
    r.append(char(msg.enable ? 1 : 0));
    // Unknown byte (copy from incoming)
    r.append(char(msg.unk));
    return r;
}
