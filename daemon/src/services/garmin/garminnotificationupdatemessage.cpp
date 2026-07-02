#include "garminnotificationupdatemessage.h"


void GarminNotificationUpdateMessage::parse() {
    // parsing not needed as no inbound message
    // just keeping the name consistent
    QByteArray msg = generateMessage();
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION UPDATE",msg);
}

QByteArray GarminNotificationUpdateMessage::generateMessage() {
    QByteArray message;

    // 1. Update type
    message.append((char)updateType);

    // 2. Category flags
    quint8 categoryFlags =NotificationTypeUtil::notificationFlags(notificationType, hasActions);
    message.append((char)categoryFlags);

    // 3. Category value
    quint8 categoryValue =
        NotificationTypeUtil::categoryValue(notificationType);
    message.append((char)categoryValue);

    // 4. Count
    message.append((char)count);

    // 5. Notification ID (LE i32)
    writeU32le(message,notificationId);


    // 6. Phone flags
    quint8 phoneFlags = 0;

    if (hasActions) {
        phoneFlags |= 0x02; // NEW_ACTIONS
    }
    if (hasPicture) {
        phoneFlags |= 0x04; // HAS_ATTACHMENTS
    }

    message.append((char)phoneFlags);

    qDebug() << Q_FUNC_INFO << " Garmin: sending notification, update type = " << (int)updateType << ", category value = " << categoryValue;
    return wrapInGfdiEnvelope((quint16)MessageId::NotificationUpdate,message);;
}
