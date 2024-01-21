#ifndef ALERTNOTIFICATIONSERVICE_H
#define ALERTNOTIFICATIONSERVICE_H

#include "qble/qbleservice.h"

/*
{00001811-0000-1000-8000-00805f9b34fb} Alert notification service
--00002a46-0000-1000-8000-00805f9b34fb //New alert
--00002a44-0000-1000-8000-00805f9b34fb //Alert notification control poiont
--00020001-78fc-48fe-8e23-433b3a1942d0 //Notification UUID used on Infinitime
*/

class AlertNotificationService : public QBLEService
{
    Q_OBJECT
public:
    AlertNotificationService(const QString &path, QObject *parent, uint8_t seperatorChar = 0x00);

    static const char* UUID_SERVICE_ALERT_NOTIFICATION;
    static const char* UUID_CHARACTERISTIC_ALERT_NOTIFICATION_NEW_ALERT;
    static const char* UUID_CHARACTERISTIC_ALERT_NOTIFICATION_CONTROL;
    static const char* UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT;


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
        CustomHuami = 250 // 0xfa
    };
    Q_ENUM(AlertCategory)

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
    Q_ENUM(HuamiIcon)

    enum AlertEvent {
        CALL_REJECT = 0,
        CALL_ANSWER = 1,
        CALL_IGNORE = 2
    };
    Q_ENUM(AlertEvent)

    static inline std::map<QString, HuamiIcon> AppToIconMap = {
        /* Facebook */
        {"facebook", HuamiIcon::FACEBOOK},
        /* Facebook Messenger */
        {"messenger", HuamiIcon::FACEBOOK_MESSENGER},
        /* Instagram */
        {"instagram", HuamiIcon::INSTAGRAM},
        /* Telegram clients */
        {"teleports.ubports_teleports", HuamiIcon::TELEGRAM},
        {"depecher", HuamiIcon::TELEGRAM},
        {"fernschreiber", HuamiIcon::TELEGRAM},
        {"sailorgram", HuamiIcon::TELEGRAM},
        {"telegram", HuamiIcon::TELEGRAM},
        {"yottogram", HuamiIcon::TELEGRAM},
        /* Twitter Clients */
        {"piepmatz", HuamiIcon::TWITTER},
        {"tweetian", HuamiIcon::TWITTER},
        {"twitter", HuamiIcon::TWITTER},
        /* Snapchat */
        {"snapchat", HuamiIcon::SNAPCHAT},
        /* Skype */
        {"skype", HuamiIcon::SKYPE},
        /* WhatsApp */
        {"whatsapp", HuamiIcon::WHATSAPP},
    };

    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message);
    Q_INVOKABLE void incomingCall(const QByteArray header, const QString &caller);
    static int mapSenderToIcon(const QString &sender);

    Q_SIGNAL void serviceEvent(const QString &c, uint8_t event);

private:
    void characteristicChanged(const QString &c, const QByteArray &value);
    uint8_t m_seperatorChar = 0x00;
};

#endif // ALERTNOTIFICATIONSERVICE_H
