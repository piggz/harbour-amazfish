#ifndef MIBANDSERVICE_H
#define MIBANDSERVICE_H

#include "qble/qbleservice.h"
#include "settingsmanager.h"
#include "bipbatteryinfo.h"
#include "logfetchoperation.h"
#include "activityfetchoperation.h"

/*
{0000FEE0-0000-1000-8000-00805f9b34fb} MiBand Service
--00002a2b-0000-1000-8000-00805f9b34fb //Current time
--00002a04-0000-1000-8000-00805f9b34fb //Peripheral Preferred Connection Parameters
--00000001-0000-3512-2118-0009af100700 //Unknown 1
--00000002-0000-3512-2118-0009af100700 //Notification
--00000003-0000-3512-2118-0009af100700 //Configuration
--00000004-0000-3512-2118-0009af100700 //Fetch data
--00000005-0000-3512-2118-0009af100700 //Activity data
--00000006-0000-3512-2118-0009af100700 //Battery Info
--00000007-0000-3512-2118-0009af100700 //Realtime steps
--00000008-0000-3512-2118-0009af100700 //User settings
--00000010-0000-3512-2118-0009af100700 //Device event
--0000000e-0000-3512-2118-0009af100700 //Weather
--00000020-0000-3512-2118-0009af100700
*/

class MiBandService : public QBLEService
{
    Q_OBJECT
public:
    MiBandService(const QString &path, QObject *parent);
    static const char* UUID_SERVICE_MIBAND;
    static const char* UUID_CHARACTERISTIC_MIBAND_CONFIGURATION;
    static const char* UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO;
    static const char* UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT;
    static const char* UUID_CHARACTERISTIC_MIBAND_NOTIFICATION;
    static const char* UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME;
    static const char* UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS;
    static const char* UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS;
    static const char* UUID_CHARACTERISTIC_MIBAND_FETCH_DATA;
    static const char* UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA;

    Q_PROPERTY(QString gpsVersion READ gpsVersion NOTIFY gpsVersionChanged())
    Q_PROPERTY(int batteryInfo READ batteryInfo NOTIFY batteryInfoChanged())
    Q_PROPERTY(int steps READ steps NOTIFY stepsChanged())

    const char RESPONSE = 0x10;
    const char SUCCESS = 0x01;
    const char COMMAND_REQUEST_GPS_VERSION = 0x0e;
    const char COMMAND_SET_LANGUAGE = 0x17;
    const char EVENT_DECLINE_CALL = 0x07;
    const char EVENT_IGNORE_CALL = 0x09;
    const char ENDPOINT_DISPLAY = 0x06;
    const char DATEFORMAT_TIME[4] = {ENDPOINT_DISPLAY, 0x0a, 0x0, 0x0 };
    const char DATEFORMAT_DATETIME[4] = {ENDPOINT_DISPLAY, 0x0a, 0x0, 0x03 };
    const char DATEFORMAT_TIME_12_HOURS[4] = {ENDPOINT_DISPLAY, 0x02, 0x0, 0x0 };
    const char DATEFORMAT_TIME_24_HOURS[4] = {ENDPOINT_DISPLAY, 0x02, 0x0, 0x1 };
    const char COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST[4] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x01};
    const char COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST[4] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x00};
    const char COMMAND_SCHEDULE_DISPLAY_ON_LIFT_WRIST[8] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const char COMMAND_ENABLE_GOAL_NOTIFICATION[4] = {ENDPOINT_DISPLAY, 0x06, 0x00, 0x01};
    const char COMMAND_DISABLE_GOAL_NOTIFICATION[4] = {ENDPOINT_DISPLAY, 0x06, 0x00, 0x00};
    const char COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO[4] = {ENDPOINT_DISPLAY, 0x0d, 0x00, 0x01};
    const char COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO[4] = {ENDPOINT_DISPLAY, 0x0d, 0x00, 0x00};
    const char COMMAND_ENABLE_DISPLAY_CALLER[5] = {ENDPOINT_DISPLAY, 0x10, 0x00, 0x00, 0x01};
    const char COMMAND_DISABLE_DISPLAY_CALLER[5] = {ENDPOINT_DISPLAY, 0x10, 0x00, 0x00, 0x00};
    const char DISPLAY_XXX[4] =  {ENDPOINT_DISPLAY, 0x03, 0x0, 0x0 };
    const char DISPLAY_YYY[5] =  {ENDPOINT_DISPLAY, 0x10, 0x0, 0x1, 0x1 };
    const char COMMAND_DISTANCE_UNIT_METRIC[4] =  { ENDPOINT_DISPLAY, 0x03, 0x00, 0x00 };
    const char COMMAND_DISTANCE_UNIT_IMPERIAL[4] =  { ENDPOINT_DISPLAY, 0x03, 0x00, 0x01 };
    const char COMMAND_SET_USERINFO = 0x4f;
    const char WEAR_LOCATION_LEFT_WRIST[4] = { 0x20, 0x00, 0x00, 0x02 };
    const char WEAR_LOCATION_RIGHT_WRIST[4] = { 0x20, 0x00, 0x00, 0x82};
    const char COMMAND_SET_FITNESS_GOAL_START[3] = { 0x10, 0x0, 0x0 };
    const char COMMAND_SET_FITNESS_GOAL_END[2] = { 0, 0 };
    const char COMMAND_ACTIVITY_DATA_START_DATE = 0x01;
    const char COMMAND_ACTIVITY_DATA_TYPE_ACTIVTY = 0x01;
    const char COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES = 0x05;
    const char COMMAND_ACTIVITY_DATA_TYPE_SPORTS_DETAILS = 0x06;
    const char COMMAND_ACTIVITY_DATA_TYPE_DEBUGLOGS = 0x07;
    const char COMMAND_FETCH_DATA = 0x02;
    const char RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS[3] = {RESPONSE, COMMAND_ACTIVITY_DATA_START_DATE, SUCCESS};
    const char RESPONSE_FINISH_SUCCESS[3] {RESPONSE, COMMAND_FETCH_DATA, SUCCESS };

    Q_INVOKABLE void requestBatteryInfo();
    Q_INVOKABLE void requestGPSVersion();
    Q_INVOKABLE QString gpsVersion();
    Q_INVOKABLE int batteryInfo();
    Q_INVOKABLE int steps() const;

    Q_INVOKABLE void setCurrentTime();
    Q_INVOKABLE void setLanguage();
    Q_INVOKABLE void setDateDisplay();
    Q_INVOKABLE void setTimeFormat();
    Q_INVOKABLE void setUserInfo();
    Q_INVOKABLE void setDistanceUnit();
    void setWearLocation(); //Not invokable because should only be done on init
    Q_INVOKABLE void setFitnessGoal();
    Q_INVOKABLE void setAlertFitnessGoal();
    Q_INVOKABLE void setEnableDisplayOnLiftWrist();

    Q_INVOKABLE void setDisplayItems();
    Q_INVOKABLE void setDoNotDisturb();
    Q_INVOKABLE void setRotateWristToSwitchInfo(bool enable);
    Q_INVOKABLE void setActivateDisplayOnLiftWrist();
    Q_INVOKABLE void setDisplayCaller();
    Q_INVOKABLE void setGoalNotification(bool enable);
    Q_INVOKABLE void setInactivityWarnings();
    Q_INVOKABLE void setHeartrateSleepSupport();
    Q_INVOKABLE void setAllDayHRM();

    //Operations
    Q_INVOKABLE void fetchLogs();
    Q_INVOKABLE void fetchActivityData();
    //Q_INVOKABLE void fetchActivitySummaries();
    //Q_INVOKABLE void fetchActivityDetail();


    Q_SIGNAL void gpsVersionChanged();
    Q_SIGNAL void batteryInfoChanged();
    Q_SIGNAL void stepsChanged();

    Q_SIGNAL void declineCall();
    Q_SIGNAL void ignoreCall();

    Q_SIGNAL void message(const QString &text);

private:
    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
    Q_SLOT void characteristicChanged(const QString &c, const QByteArray &value);
    void setGPSVersion(const QString& v);
    void handleFetchLogMetaData(const QByteArray &value);
    void handleFetchActivityMetaData(const QByteArray &value);

    QDateTime lastActivitySync();

    QString m_gpsVersion;
    int m_steps;
    int m_operationRunning = 0;

    SettingsManager m_settings;
    BipBatteryInfo m_batteryInfo;
    LogFetchOperation *m_logFetchOperation = nullptr;
    ActivityFetchOperation *m_activityFetchOperation = nullptr;
};

#endif // MIBANDSERVICE_H
