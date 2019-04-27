#ifndef DAEMONINTERFACE_H
#define DAEMONINTERFACE_H

#include <QObject>

class DaemonInterface : public QObject
{
    Q_OBJECT
public:
    explicit DaemonInterface(QObject *parent = nullptr);

    enum ActivityType {
        NotMeasured = -1,
        Unknown = 0,
        Activity = 1,
        LightSleep = 2,
        DeepSleep = 4,
        NotWorn = 8,
        Running = 16,
        Walking = 32,
        Swimming = 64,
        Biking = 128,
        Treadmill = 256
    };
    Q_ENUM(ActivityType)

    enum Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER,
        FEATURE_ACTIVITY,
        FEATURE_STEPS,
        FEATURE_ALARMS,
        FEATURE_ALERT,
        FEATURE_NOTIFIATION
    };
    Q_ENUM(Feature)

    enum Info {
        INFO_SWVER = 1,
        INFO_HWVER,
        INFO_SERIAL,
        INFO_SYSTEMID,
        INFO_PNPID,
        INFO_GPSVER,
        INFO_BATTERY,
        INFO_STEPS,
        INFO_HEARTRATE
    };
    Q_ENUM(Info)

    enum Settings {
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
        SETTING_DEVICE_UNIT
    };
    Q_ENUM(Settings)

    static QString activityToString(ActivityType type);

signals:

public slots:
};

#endif // DAEMONINTERFACE_H
