#ifndef MIBANDSERVICE_H
#define MIBANDSERVICE_H

#include "qble/qbleservice.h"
#include "bipbatteryinfo.h"
#include "logfetchoperation.h"
#include "activityfetchoperation.h"
#include "sportssummaryoperation.h"
#include "sportsdetailoperation.h"
#include "weather/currentweather.h"
#include "devices/abstractdevice.h"

#include <QTimer>
#include <QtCore/QJsonDocument>

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
--00000016-0000-3512-2118-0009af100700 //2021 Chunked Char Write
--00000017-0000-3512-2118-0009af100700 //2021 Chunked Char Read
--00000020-0000-3512-2118-0009af100700 //Chunked transfer
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
    static const char* UUID_CHARACTERISTIC_MIBAND_WEATHER;
    static const char* UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER;
    static const char* UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE;
    static const char* UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ;

    static const uint8_t RESPONSE = 0x10;
    static const uint8_t SUCCESS = 0x01;
    static const uint8_t FAIL = 0x04;

    static const uint8_t EVENT_BUTTON = 0x04;
    static const uint8_t EVENT_DECLINE_CALL = 0x07;
    static const uint8_t EVENT_IGNORE_CALL = 0x09;
    static const uint8_t EVENT_FIND_PHONE = 0x08;
    static const uint8_t EVENT_CANCEL_FIND_PHONE = 0x0f;
    static const uint8_t MTU_REQUEST = 0x16;
    static const uint8_t EVENT_MUSIC = 0xfe;
    static const uint8_t EVENT_MUSIC_OPEN = 0xe0;
    static const uint8_t EVENT_MUSIC_PLAY = 0x00;
    static const uint8_t EVENT_MUSIC_PAUSE = 0x01;
    static const uint8_t EVENT_MUSIC_NEXT = 0x03;
    static const uint8_t EVENT_MUSIC_PREV = 0x04;
    static const uint8_t EVENT_MUSIC_VOLUP = 0x05;
    static const uint8_t EVENT_MUSIC_VOLDOWN = 0x06;

    static const uint8_t ENDPOINT_DISPLAY = 0x06;
    static const uint8_t ENDPOINT_DISPLAY_ITEMS = 0x0a;

    static constexpr uint8_t DATEFORMAT_TIME[4] = {ENDPOINT_DISPLAY, 0x0a, 0x0, 0x0 };
    static constexpr uint8_t DATEFORMAT_DATETIME[4] = {ENDPOINT_DISPLAY, 0x0a, 0x0, 0x03 };
    static constexpr uint8_t DATEFORMAT_TIME_12_HOURS[4] = {ENDPOINT_DISPLAY, 0x02, 0x0, 0x0 };
    static constexpr uint8_t DATEFORMAT_TIME_24_HOURS[4] = {ENDPOINT_DISPLAY, 0x02, 0x0, 0x1 };

    static const uint8_t COMMAND_REQUEST_ALARMS = 0x0d;
    static const uint8_t COMMAND_REQUEST_GPS_VERSION = 0x0e;
    static const uint8_t COMMAND_SET_LANGUAGE = 0x17;
    static constexpr uint8_t COMMAND_ENABLE_DISPLAY_ON_LIFT_WRIST[4] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x01};
    static constexpr uint8_t COMMAND_DISABLE_DISPLAY_ON_LIFT_WRIST[4] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x00};
    static constexpr uint8_t COMMAND_SCHEDULE_DISPLAY_ON_LIFT_WRIST[8] = {ENDPOINT_DISPLAY, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
    static constexpr uint8_t COMMAND_ENABLE_GOAL_NOTIFICATION[4] = {ENDPOINT_DISPLAY, 0x06, 0x00, 0x01};
    static constexpr uint8_t COMMAND_DISABLE_GOAL_NOTIFICATION[4] = {ENDPOINT_DISPLAY, 0x06, 0x00, 0x00};
    static constexpr uint8_t COMMAND_ENABLE_ROTATE_WRIST_TO_SWITCH_INFO[4] = {ENDPOINT_DISPLAY, 0x0d, 0x00, 0x01};
    static constexpr uint8_t COMMAND_DISABLE_ROTATE_WRIST_TO_SWITCH_INFO[4] = {ENDPOINT_DISPLAY, 0x0d, 0x00, 0x00};
    static constexpr uint8_t COMMAND_ENABLE_DISPLAY_CALLER[5] = {ENDPOINT_DISPLAY, 0x10, 0x00, 0x00, 0x01};
    static constexpr uint8_t COMMAND_DISABLE_DISPLAY_CALLER[5] = {ENDPOINT_DISPLAY, 0x10, 0x00, 0x00, 0x00};
    static constexpr uint8_t COMMAND_DISTANCE_UNIT_METRIC[4] =  { ENDPOINT_DISPLAY, 0x03, 0x00, 0x00 };
    static constexpr uint8_t COMMAND_DISTANCE_UNIT_IMPERIAL[4] =  { ENDPOINT_DISPLAY, 0x03, 0x00, 0x01 };
    static const uint8_t COMMAND_SET_USERINFO = 0x4f;
    static constexpr uint8_t COMMAND_SET_FITNESS_GOAL_START[3] = { 0x10, 0x0, 0x0 };
    static constexpr uint8_t COMMAND_SET_FITNESS_GOAL_END[2] = { 0, 0 };
    static const uint8_t COMMAND_ACTIVITY_DATA_START_DATE = 0x01;
    static const uint8_t COMMAND_ACTIVITY_DATA_TYPE_ACTIVTY = 0x01;
    static const uint8_t COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES = 0x05;
    static const uint8_t COMMAND_ACTIVITY_DATA_TYPE_SPORTS_DETAILS = 0x06;
    static const uint8_t COMMAND_ACTIVITY_DATA_TYPE_DEBUGLOGS = 0x07;
    static const uint8_t COMMAND_FETCH_DATA = 0x02;
    static constexpr uint8_t COMMAND_CHANGE_SCREENS[12] = {ENDPOINT_DISPLAY_ITEMS, 0x01, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    static constexpr uint8_t COMMAND_ENABLE_DISCONNECT_NOTIFICATION[8] = {ENDPOINT_DISPLAY, 0x0c, 0x00, 0x01, 0, 0, 0, 0};
    static constexpr uint8_t COMMAND_DISABLE_DISCONNECT_NOTIFICATION[8] = {ENDPOINT_DISPLAY, 0x0c, 0x00, 0x00, 0, 0, 0, 0};


    static constexpr uint8_t DISPLAY_XXX[4] =  {ENDPOINT_DISPLAY, 0x03, 0x0, 0x0 };
    static constexpr uint8_t DISPLAY_YYY[5] =  {ENDPOINT_DISPLAY, 0x10, 0x0, 0x1, 0x1 };

    static constexpr uint8_t WEAR_LOCATION_LEFT_WRIST[4] = { 0x20, 0x00, 0x00, 0x02 };
    static constexpr uint8_t WEAR_LOCATION_RIGHT_WRIST[4] = { 0x20, 0x00, 0x00, 0x82};

    static constexpr uint8_t RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS[3] = {RESPONSE, COMMAND_ACTIVITY_DATA_START_DATE, SUCCESS};
    static constexpr uint8_t RESPONSE_FINISH_SUCCESS[3] {RESPONSE, COMMAND_FETCH_DATA, SUCCESS };
    static constexpr uint8_t RESPONSE_FINISH_FAIL[3] {RESPONSE, COMMAND_FETCH_DATA, FAIL };

    void requestBatteryInfo();
    void requestGPSVersion();
    QString gpsVersion();
    int batteryInfo();
    int steps() const;

    void setCurrentTime();
    void setLanguage();
    void setDateDisplay();
    void setTimeFormat();
    void setUserInfo();
    void setDistanceUnit();
    void setWearLocation();
    void setFitnessGoal();
    void setAlertFitnessGoal();
    void setEnableDisplayOnLiftWrist();
    void setDisplayItems();
    void setDisplayItemsOld(QMap<QString, uint8_t> keyPosMap);
    void setDisplayItemsNew();
    void setDoNotDisturb();
    void setRotateWristToSwitchInfo(bool enable);
    void setDisplayCaller();
    void setInactivityWarnings();
    void setAlarms();
    void requestAlarms();
    void setDisconnectNotification();

    void sendWeather(const CurrentWeather *weather, bool supportsConditionString);

    //Operations
    void fetchLogs();
    void fetchActivityData();
    void fetchSportsSummaries();
    //Q_INVOKABLE void fetchActivityDetail();

    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString &val);
    Q_SIGNAL void buttonPressed();
    Q_SIGNAL void serviceEvent(uint8_t event);

    virtual bool operationRunning() override;
    void abortOperations();

    void setDatabase(KDbConnection *conn);

    void writeChunked(const QString &characteristic, int type, const QByteArray &value);

    void sendAlert(const QString &sender, const QString &subject, const QString &message);

    void setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration, int position);

private:
    void characteristicRead(const QString &c, const QByteArray &value);
    void characteristicChanged(const QString &c, const QByteArray &value);
    void operationTimeout();

    void setGPSVersion(const QString& v);
    void decodeAlarms(const QByteArray &data);

    QString m_gpsVersion;
    int m_steps = 0;
    int m_operationRunning = 0;

    BipBatteryInfo m_batteryInfo;
    LogFetchOperation *m_logFetchOperation = nullptr;
    ActivityFetchOperation *m_activityFetchOperation = nullptr;
    SportsSummaryOperation *m_sportsSummaryOperation = nullptr;
    SportsDetailOperation *m_sportsDetailOperation = nullptr;

    KDbConnection *m_conn = nullptr;
    QTimer *m_operationTimeout = nullptr;
    QMap<QString, uint8_t> displayItemsIdMap;
};

#endif // MIBANDSERVICE_H
