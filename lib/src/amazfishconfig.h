#ifndef AMAZFISHCONFIG_H
#define AMAZFISHCONFIG_H

#include <QObject>
#include <QVariant>
#include <QDate>

#ifdef MER_EDITION_SAILFISH
class MDConfGroup;
#else
#include <QSettings>
#endif
class QQmlEngine;
class QJSEngine;

#define OPTION(key, name, setter, def, type, gettertype, settertype)  \
    Q_PROPERTY(type name READ name WRITE setter NOTIFY name##Changed) \
    type name() const                                                 \
    { return value(key, def).gettertype(); }                          \
    void setter(settertype value)                                     \
    { setValue(key, value, &AmazfishConfig::name##Changed); }         \
    Q_SIGNAL void name##Changed();

#define BOOL_OPTION(key, name, setter, def)   \
    OPTION(key, name, setter, def, bool, toBool, bool)

#define INT_OPTION(key, name, setter, def)    \
    OPTION(key, name, setter, def, int, toInt, int)

#define UINT_OPTION(key, name, setter, def)   \
    OPTION(key, name, setter, def, uint, toInt, uint)

#define INT64_OPTION(key, name, setter, def)  \
    OPTION(key, name, setter, def, qint64, toLongLong, qint64)

#define STRING_OPTION(key, name, setter, def) \
    OPTION(key, name, setter, def, QString, toString, const QString &)

#define ENUM_OPTION(key, name, setter, type, def)                               \
    Q_PROPERTY(type name READ name WRITE setter NOTIFY name##Changed)           \
    type name() const                                                           \
    { return static_cast<type>(value(key, def).toInt()); }                      \
    void setter(type value)                                                     \
    { setValue(key, static_cast<int>(value), &AmazfishConfig::name##Changed); } \
    Q_SIGNAL void name##Changed();

#define ALARM_OPTION(key, name, setter, type, def, gettertype) \
    Q_INVOKABLE type name(quint8 n) const                      \
    { return value(key.arg(n), def).gettertype(); }            \
    Q_INVOKABLE void setter(quint8 n, type value)              \
    { setValue(key.arg(n), value); }


class AmazfishConfig : public QObject
{
    Q_OBJECT

    AmazfishConfig(QObject *parent = nullptr);

public:
    enum DeviceLanguage {
        DeviceLanguageEnUs,
        DeviceLanguageEsEs,
        DeviceLanguageZhCn,
        DeviceLanguageZhTw,
        DeviceLanguageRuRu,
        DeviceLanguageDeDe,
        DeviceLanguageItIt,
        DeviceLanguageFrFr,
        DeviceLanguageTrTr,
    };
    Q_ENUM(DeviceLanguage)

    enum DeviceDateFormat {
        DeviceDateFormatTime,
        DeviceDateFormatDateTime,
    };
    Q_ENUM(DeviceDateFormat)

    enum DeviceTimeFormat {
        DeviceTimeFormat24H,
        DeviceTimeFormat12H,
    };
    Q_ENUM(DeviceTimeFormat)

    enum DeviceDistanceUnit {
        DeviceDistanceUnitMetric,
        DeviceDistanceUnitImperial,
    };
    Q_ENUM(DeviceDistanceUnit)

    enum ProfileGender {
        ProfileGenderMale,
        ProfileGenderFemale,
    };
    Q_ENUM(ProfileGender)

    enum WearLocation {
        WearLocationLeftWrist,
        WearLocationRightWrist,
    };
    Q_ENUM(WearLocation)

    enum DisplayLiftWrist {
        DisplayLiftWristOff,
        DisplayLiftWristOn,
        DisplayLiftWristSchedule
    };
    Q_ENUM(DisplayLiftWrist)

    static AmazfishConfig *instance();
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine)
        Q_UNUSED(scriptEngine)
        return instance();
    }

    QVariant value(const QString &key, const QVariant &def = QVariant()) const;
    void setValue(const QString &key, const QVariant &value);

    STRING_OPTION(QStringLiteral("pairedAddress"), pairedAddress, setPairedAddress, QString())
    STRING_OPTION(QStringLiteral("pairedName"),    pairedName,    setPairedName,    QString())

    BOOL_OPTION(QStringLiteral("app/notifyconnect"),    appNotifyConnect,    setAppNotifyConnect,    true)
    BOOL_OPTION(QStringLiteral("app/autosyncdata"),     appAutoSyncData,     setAppAutoSyncData,     true)
    BOOL_OPTION(QStringLiteral("app/notifylowbattery"), appNotifyLowBattery, setAppNotifyLowBattery, true)
    BOOL_OPTION(QStringLiteral("app/overridefwcheck"),  appOverrideFwCheck,  setAppOverrideFwCheck,  false)
    BOOL_OPTION(QStringLiteral("app/navigationnotification"),  appNavigationNotification,  setAppNavigationNotification,  false)
    BOOL_OPTION(QStringLiteral("app/simulateevents"),  appSimulateEventSupport,  setSimulateEventSupport,  false)
    BOOL_OPTION(QStringLiteral("app/transliterate"),  appTransliterate,  setTransliterate,  false)

    STRING_OPTION(QStringLiteral("app/button-double-action"),    appButtonDoublePressAction,    setAppButtonDoublePressAction,    "action-none")
    STRING_OPTION(QStringLiteral("app/button-triple-action"),    appButtonTriplePressAction,    setAppButtonTriplePressAction,    "action-none")
    STRING_OPTION(QStringLiteral("app/button-quad-action"),    appButtonQuadPressAction,    setAppButtonQuadPressAction,    "action-none")

    STRING_OPTION(QStringLiteral("localAdapter"),    localAdapter,    setLocalAdapter,    "/org/bluez/hci0")

    INT_OPTION(QStringLiteral("app/refreshweather"),    appRefreshWeather,   setAppRefreshWeather,   80)
    INT_OPTION(QStringLiteral("app/refreshcalendar"),   appRefreshCalendar,  setAppRefreshCalendar,  60)

    BOOL_OPTION(QStringLiteral("device/realtimehrmmeasurement"), deviceRealtimeHRMMeasurement, setdeviceRealtimeHRMMeasurement, false)
    BOOL_OPTION(QStringLiteral("device/disconnectnotification"), deviceDisconnectNotification, setDeviceDisconnectNotification, false)
    BOOL_OPTION(QStringLiteral("device/displayweathershortcut"), deviceDisplayWeatherShortcut, setDeviceDisplayWeatherShortcut, true)
    BOOL_OPTION(QStringLiteral("device/displayalipayshortcut"),  deviceDisplayAliPayShortcut,  setDeviceDisplayAliPayShortcut,  true)
    STRING_OPTION(QStringLiteral("device/displayitems"),         deviceDisplayItems,           setDeviceDisplayItems,           "")

    ENUM_OPTION(QStringLiteral("device/language"),     deviceLanguage,     setDeviceLanguage,     DeviceLanguage,     DeviceLanguageEnUs)
    ENUM_OPTION(QStringLiteral("device/dateformat"),   deviceDateFormat,   setDeviceDateFormat,   DeviceDateFormat,   DeviceDateFormatTime)
    ENUM_OPTION(QStringLiteral("device/timeformat"),   deviceTimeFormat,   setDeviceTimeFormat,   DeviceTimeFormat,   DeviceTimeFormat24H)
    ENUM_OPTION(QStringLiteral("device/distanceunit"), deviceDistanceUnit, setDeviceDistanceUnit, DeviceDistanceUnit, DeviceDistanceUnitMetric)

    INT64_OPTION(QStringLiteral("device/lastactivitysyncmillis"), lastActivitySync, setLastActivitySync, 0)
    INT64_OPTION(QStringLiteral("device/lastsportsyncmillis"),    lastSportSync,    setLastSportSync,    0)

    STRING_OPTION(QStringLiteral("device/authkey"), deviceAuthKey, setDeviceAuthKey, QString())

    STRING_OPTION(QStringLiteral("profile/name"), profileName, setProfileName, QString())
    OPTION(QStringLiteral("profile/dob"), profileDOB, setProfileDOB, QDateTime(), QDateTime, toDateTime, const QDateTime &)

    ENUM_OPTION(QStringLiteral("profile/gender"),       profileGender,       setProfileGender,       ProfileGender, ProfileGenderMale)
    ENUM_OPTION(QStringLiteral("profile/wearlocation"), profileWearLocation, setProfileWearLocation, WearLocation,  WearLocationLeftWrist)

    UINT_OPTION(QStringLiteral("profile/height"),      profileHeight,      setProfileHeight,      200)
    UINT_OPTION(QStringLiteral("profile/weight"),      profileWeight,      setProfileWeight,      70)
    UINT_OPTION(QStringLiteral("profile/alldayhrm"),   profileAllDayHRM,   setProfileAllDayHRM,   0)
    UINT_OPTION(QStringLiteral("profile/fitnessgoal"), profileFitnessGoal, setProfileFitnessGoal, 10000)

    BOOL_OPTION(QStringLiteral("profile/alertfitnessgoal"),   profileAlertFitnessGoal,   setProfileAlertFitnessGoal,   false)
    BOOL_OPTION(QStringLiteral("profile/hrmsleepsupport"),    profileHRMSleepSupport,    setProfileHRMSleepSupport,    true)
    ENUM_OPTION(QStringLiteral("profile/displayonliftwrist"), profileDisplayOnLiftWrist, setProfileDisplayOnLiftWrist, DisplayLiftWrist, DisplayLiftWristOn)
    OPTION(QStringLiteral("profile/wristScheduleStart"), profileWristScheduleStart, setProfileWristScheduleStart, QTime(), QDateTime, toDateTime, const QDateTime &)
    OPTION(QStringLiteral("profile/wristScheduleEnd"), profileWristScheduleEnd, setProfileWristScheduleEnd, QTime(), QDateTime, toDateTime, const QDateTime &)

    ALARM_OPTION(QStringLiteral("alarms/alarm%1/enabled"), alarmEnabled,    setAlarmEnabled,    bool, false, toBool)
    ALARM_OPTION(QStringLiteral("alarms/alarm%1/repeat"),  alarmRepeatMask, setAlarmRepeatMask, int,  0,     toInt)
    ALARM_OPTION(QStringLiteral("alarms/alarm%1/hour"),    alarmHour,       setAlarmHour,       int,  0,     toInt)
    ALARM_OPTION(QStringLiteral("alarms/alarm%1/minute"),  alarmMinute,     setAlarmMinute,     int,  0,     toInt)

private:
    using signal_ptr = void(AmazfishConfig::*)();

    void setValue(const QString &key, const QVariant &value, signal_ptr signal);

#ifdef MER_EDITION_SAILFISH
    MDConfGroup *m_group;
#endif
};

#undef ALARM_OPTION
#undef ENUM_OPTION
#undef STRING_OPTION
#undef INT64_OPTION
#undef UINT_OPTION
#undef INT_OPTION
#undef BOOL_OPTION
#undef OPTION

#endif // AMAZFISHCONFIG_H
