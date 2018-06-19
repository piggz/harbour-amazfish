#ifndef ALERTNOTIFICATIONSERVICE_H
#define ALERTNOTIFICATIONSERVICE_H

#include "qble/qbleservice.h"

/*
{00001811-0000-1000-8000-00805f9b34fb} Alert notification service
--00002a46-0000-1000-8000-00805f9b34fb //New alert
--00002a44-0000-1000-8000-00805f9b34fb //Alert notification control poiont
*/

class AlertNotificationService : public QBLEService
{
    Q_OBJECT
public:
    AlertNotificationService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_ALERT_NOTIFICATION;
    static const char* UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT;
    static const char* UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL;

    Q_ENUMS(AlertCategory)
    enum AlertCategory {
        Simple = 0,
        Email = 1,
        News = 2,
        IncomingCall = 3,
        MissedCall = 4,
        SMS = 5,
        VoiceMail = 6,
        Schedule = 7,
        HighPriorityAlert = 8,
        InstantMessage = 9,
        // 10-250 reserved for future use
        // 251-255 defined by service specification
        Any = 255,
        Custom = -1,
        CustomHuami = 250
    };

    Q_ENUMS(HuamiIcon)
    enum HuamiIcon {
        WECHAT = 0,
        PENGUIN_1 = 1,
        MI_CHAT_2 = 2,
        FACEBOOK = 3,
        TWITTER = 4,
        MI_APP_5 = 5,
        SNAPCHAT = 6,
        WHATSAPP = 7,
        RED_WHITE_FIRE_8 = 8,
        CHINESE_9 = 9,
        ALARM_CLOCK = 10,
        APP_11 = 11,
        INSTAGRAM = 12,
        CHAT_BLUE_13 = 13,
        COW_14 = 14,
        CHINESE_15 = 15,
        CHINESE_16 = 16,
        STAR_17 = 17,
        APP_18 = 18,
        CHINESE_19 = 19,
        CHINESE_20 = 20,
        CALENDAR = 21,
        FACEBOOK_MESSENGER = 22,
        VIBER = 23,
        LINE = 24,
        TELEGRAM = 25,
        KAKAOTALK = 26,
        SKYPE = 27,
        VKONTAKTE = 28,
        POKEMONGO = 29,
        HANGOUTS = 30,
        MI_31 = 31,
        CHINESE_32 = 32,
        CHINESE_33 = 33,
        EMAIL = 34,
        WEATHER = 35,
        HR_WARNING_36 = 36
    };

    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);

private:
    int mapSenderToIcon(const QString &sender);
    int m_lastAlertHash = 0;
};

#endif // ALERTNOTIFICATIONSERVICE_H
