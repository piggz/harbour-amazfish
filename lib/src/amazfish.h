#ifndef AMAZFISH_H
#define AMAZFISH_H

#include <QObject>
#include <QVariantMap>

//Container class for enums
class Amazfish : public QObject {
    Q_OBJECT
public:
    enum class Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER = 2,
        FEATURE_ACTIVITY = 4,
        FEATURE_STEPS = 8,
        FEATURE_ALARMS = 16,
        FEATURE_ALERT = 32,
        FEATURE_EVENT_REMINDER = 64,
        FEATURE_MUSIC_CONTROL = 128,
        FEATURE_BUTTON_ACTION = 256,
        FEATURE_SCREENSHOT = 512,
    };
    Q_ENUM(Feature)

    enum class Info {
        INFO_SWVER = 1,
        INFO_HWVER,
        INFO_SERIAL,
        INFO_SYSTEMID,
        INFO_PNPID,
        INFO_GPSVER,
        INFO_BATTERY,
        INFO_STEPS,
        INFO_HEARTRATE,
        INFO_MODEL,
        INFO_FW_REVISION,
        INFO_MANUFACTURER
    };
    Q_ENUM(Info)

    enum class Settings {
        SETTING_USER_PROFILE,
        SETTING_USER_GOAL,
        SETTING_USER_ALERT_GOAL,
        SETTING_USER_ALL_DAY_HRM,
        SETTING_USER_HRM_SLEEP_DETECTION,
        SETTING_USER_DISPLAY_ON_LIFT,
        SETTING_ALARMS,
        SETTING_DEVICE_DISPLAY_ITEMS,
        SETTING_DEVICE_LANGUAGE,
        SETTING_DEVICE_DATE,
        SETTING_DEVICE_TIME,
        SETTING_DEVICE_UNIT,
        SETTING_DISCONNECT_NOTIFICATION,
        SETTING_DEVICE_REALTIME_HRM_MEASUREMENT
    };
    Q_ENUM(Settings)

    struct WatchNotification
    {
        int id;
        QString appId;
        QString appName;
        QString summary;
        QString body;

        QVariantMap toQVariantMap() const {
            QVariantMap map;
            map["id"] = id;
            map["appId"] = appId;
            map["appName"] = appName;
            map["summary"] = summary;
            map["body"] = body;
            return map;
        }

        static WatchNotification fromQVariantMap(const QVariantMap &map) {
            WatchNotification n;
            n.id = map.value("id").toInt();
            n.appId = map.value("appId").toString();
            n.appName = map.value("appName").toString();
            n.summary = map.value("summary").toString();
            n.body = map.value("body").toString();
            return n;
        }
    };


};
Q_DECLARE_METATYPE(Amazfish::Feature)
Q_DECLARE_METATYPE(Amazfish::Info)
Q_DECLARE_METATYPE(Amazfish::Settings)
Q_DECLARE_METATYPE(Amazfish::WatchNotification)

#endif // AMAZFISH_H
