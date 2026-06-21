#include "garminnotificationupdatemessage.h"


void GarminNotificationUpdateMessage::parse() {
    // parsing not needed as no inbound message
    // just keeping the name consistent
    QByteArray msg = generateMessage();
    msg=wrapInGfdiEnvelope(5033,msg);
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION UPDATE",msg);
}

QByteArray GarminNotificationUpdateMessage::generateMessage() {
    QByteArray message;

    // 1. Update type
    message.append(static_cast<char>(updateType));

    // 2. Category flags
    quint8 categoryFlags =NotificationTypeUtil::notificationFlags(notificationType, hasActions);
    message.append(static_cast<char>(categoryFlags));

    // 3. Category value
    quint8 categoryValue =
        NotificationTypeUtil::categoryValue(notificationType);
    message.append(static_cast<char>(categoryValue));

    // 4. Count
    message.append(static_cast<char>(count));

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

    message.append(static_cast<char>(phoneFlags));

    return message;
}
