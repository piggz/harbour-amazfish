#include "garminnotificationsubscriptionmessage.h"
#include "garmintypes.h"


void GarminNotificationSubscriptionMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing notification subscription. Data = " <<  data.toHex();
    if (data.size() < 2) {
        return;
    }

    mMessage.enable = (quint8(data[0]) == 1);
    if (data.size()>1) mMessage.unk = quint8(data[1]); else mMessage.unk=0;
    // First send subscription status message as ACK
    QByteArray response = generateStatusMessage();
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION SUBSCRIPTION RESPONSE",response);
    // no other response message needed
}

QByteArray GarminNotificationSubscriptionMessage::generateStatusMessage() {
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
    r.append(char(mMessage.enable ? 1 : 0));
    // Unknown byte (copy from incoming)
    r.append(char(mMessage.unk));
    return wrapInGfdiEnvelope((quint16)MessageId::Response,r);

}

QByteArray GarminNotificationSubscriptionStatusMessage::getOutgoingMessage() {
    QByteArray response;
    writeU16le(response,(quint16)getMessageType());
    response.append((char)mStatus);
    response.append(mNotificationStatus ? 0:1);
    response.append(mEnableRaw ? 1:0);
    response.append(mUnk);
    return wrapInGfdiEnvelope((quint16)MessageId::Response,response);
}
