#include "garminnotificationupdatemessage.h"

enum class NotificationFlag : quint8 {
     BACKGROUND = 0,
     FOREGROUND = 1,
     UNK = 2,
     ACTION_ACCEPT= 3, //only needed for legacy actions
     ACTION_DECLINE = 4, //only needed for legacy actions
 };

 enum class NotificationCategory : quint8 { //was AncsCategory
     OTHER = 0,
     INCOMING_CALL = 1,
     MISSED_CALL = 2,
     VOICEMAIL = 3,
     SOCIAL = 4,
     SCHEDULE = 5,
     EMAIL = 6,
     NEWS = 7,
     HEALTH_AND_FITNESS = 8,
     BUSINESS_AND_FINANCE = 9,
     LOCATION= 10,
     ENTERTAINMEN = 11,
     SMS = 12
 };

 enum class NotificationPhoneFlags : quint8 {
     LEGACY_ACTIONS = 0,
     NEW_ACTIONS = 1,
     HAS_ATTACHMENTS = 2
 };

quint8 GarminNotificationUpdateMessage::getCategoryValue() {
    switch (notificationType) {
        case NotificationType::GenericPhone:
            return (quint8)NotificationCategory::INCOMING_CALL;
        case NotificationType::GenericEmail:
            return (quint8)NotificationCategory::EMAIL;
        case NotificationType::GenericSms:
        case NotificationType::GenericChat:
            return (quint8)NotificationCategory::SMS;
        case NotificationType::GenericNavigation:
            return (quint8)NotificationCategory::LOCATION;
        case NotificationType::GenericSocial:
            return (quint8)NotificationCategory::SOCIAL;
        case NotificationType::GenericAlarmClock:
        case NotificationType::Generic:
        default:
            return (quint8)NotificationCategory::OTHER;
    }
    return (quint8)NotificationCategory::OTHER;
}

quint8 GarminNotificationUpdateMessage::getCategoryFlags() {

    quint8 flags = 0;
    /*
    if (this.hasActions && this.useLegacyActions) { //only needed for legacy actions
        flags.add(NotificationFlag.ACTION_ACCEPT);
    }
    */
    flags = flags | (0x01) << (quint8)NotificationFlag::ACTION_DECLINE;

    switch (notificationType) {
        case NotificationType::GenericPhone:
        case NotificationType::GenericEmail:
        case NotificationType::GenericSms:
        case NotificationType::GenericChat:
            flags |= 0x01 << (quint8)NotificationFlag::FOREGROUND;
            break;
        case NotificationType::GenericNavigation:
        case NotificationType::GenericSocial:
        case NotificationType::GenericAlarmClock:
        case NotificationType::Generic:
            // TODO: Maybe make this configurable, but most users expect all notifications
            // to be foreground, sending them as background was generating bug reports.
            flags |= 0x01 << (quint8)NotificationFlag::FOREGROUND;
    }
    return flags;
}

quint8 GarminNotificationUpdateMessage::getNotificationPhoneFlags() {
    quint8 flags = 0;
    if (hasActions)
        flags |= 0x01 << (quint8)NotificationPhoneFlags::NEW_ACTIONS;
    /*
    if (usLegacyActions)
        flags |= 0x01 << (quint8)NotificationPhoneFlags::LEGACY_ACTIONS;
    */
    if (hasPicture)
        flags |= 0x01 << (quint8)NotificationPhoneFlags::HAS_ATTACHMENTS;
    return flags;

}

void GarminNotificationUpdateMessage::parse() {
    // parsing not needed as no inbound message
    // just keeping the name consistent
    QByteArray msg = generateMessage();
    if (mCommunicator) mCommunicator->sendMessage("NOTIFICATION UPDATE",msg);
}

QByteArray GarminNotificationUpdateMessage::generateMessage() {
    QByteArray message;
    // NotificationUpdateMessage(ADD): [type][categoryFlags][categoryValue][count][id:4][phoneFlags]
    // categoryFlags = FOREGROUND(bit1) | ACTION_DECLINE(bit4) = 0x12 (same for every generic
    // category per NotificationUpdateMessage.getCategoryFlags()), category = SMS(12) - some
    // Garmin notification filters allow Calls/Texts by default but suppress generic "Other"
    // app notifications, so SMS is the safest default until per-category mapping is verified
    // against real hardware.

    // 1. Update type
    message.append((char)updateType);

    // 2. Category flags
    //quint8 categoryFlags =NotificationTypeUtil::notificationFlags(notificationType, hasActions);
    quint8 categoryFlags = getCategoryFlags();
    message.append(categoryFlags);

    // 3. Category value
    quint8 categoryValue = getCategoryValue();
    message.append(categoryValue);

    // 4. Count
    message.append((char)count);

    // 5. Notification ID (LE i32)
    writeU32le(message,notificationId);

    // 6. Phone flags
    quint8 phoneFlags = getNotificationPhoneFlags();
    message.append(phoneFlags);

    qDebug() << Q_FUNC_INFO << " Garmin: sending notification, update type = " << (int)updateType << ", category value = " << categoryValue;
    return wrapInGfdiEnvelope(MessageId::NotificationUpdate,message);
}
