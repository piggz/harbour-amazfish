#include "garminnotificationsubscriptionmessage.h"


void GarminNotificationSubscriptionMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification subscription. Data = " <<  data.toHex();
    if (data.size() < 2) {
        return;
    }
    NotificationSubscriptionMessage msg;
    msg.enable = (quint8(data[0]) == 1);
    if (data.size()>1) msg.unk = quint8(data[1]); else msg.unk=0;
    // First send subscription status message as ACK
    QByteArray response = generateStatusMessage(msg);
    response = wrapInGfdiEnvelope(5000,response);
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION SUBSCRIPTION RESPONSE",response);
    // no other response message needed
}

QByteArray GarminNotificationSubscriptionMessage::generateStatusMessage(NotificationSubscriptionMessage& msg) {
    QByteArray r;
    bool enabled =true;
    //todo: set notification handler enable in device
    //Todo: Check if notfications are enabled in device settings, setting to true for now.
    // could be in AmazfishConfig::deviceDisconnectNotification() ?

    writeU16le(r,(quint16)MessageId::NotificationSubscription);
    r.append(char(Status::Ack));
    // Notification Status (0 = ENABLED, 1 = DISABLED)
    r.append(char(enabled) ? 0 : 1);
    // Enable flag (matches incoming request)
    r.append(char(msg.enable ? 1 : 0));
    // Unknown byte (copy from incoming)
    r.append(char(msg.unk));
    return r;

}
