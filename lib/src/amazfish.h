#ifndef AMAZFISH_H
#define AMAZFISH_H

#include <QObject>

//Container class for enums
class Amazfish : public QObject {
    Q_OBJECT
public:
    enum class ActivityType {
        NotMeasured = -1,
        Unknown = 0x00000000,
        Activity = 0x00000001,
        LightSleep = 0x00000002,
        DeepSleep = 0x00000004,
        NotWorn = 0x00000008,
        Running = 0x00000010,
        Walking = 0x00000020,
        Swimming = 0x00000040,
        Cycling = 0x00000080,
        Treadmill = 0x00000100,
        Exercise = 0x00000200,
        OpenSwimming = 0x00000400,
        IndoorCycling = 0x00000800,
        EllipticalTrainer = 0x00001000,
        JumpRope = 0x00002000,
        Yoga = 0x00004000,
        TrailRunning = 0x00008000,
        Skiing = 0x00010000
    };
    Q_ENUM(ActivityType)

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
        SETTING_DISCONNECT_NOTIFICATION
    };
    Q_ENUM(Settings)

    static QString activityToString(Amazfish::ActivityType type);
};
Q_DECLARE_METATYPE(Amazfish::ActivityType)
Q_DECLARE_METATYPE(Amazfish::Feature)
Q_DECLARE_METATYPE(Amazfish::Info)
Q_DECLARE_METATYPE(Amazfish::Settings)

#endif // AMAZFISH_H
