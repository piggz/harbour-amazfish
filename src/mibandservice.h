#ifndef MIBANDSERVICE_H
#define MIBANDSERVICE_H

#include "bipservice.h"
#include "settingsmanager.h"

/*
{0000FEE0-0000-1000-8000-00805f9b34fb} MiBand Service
--00002a2b-0000-1000-8000-00805f9b34fb //Current time
--00002a04-0000-1000-8000-00805f9b34fb //Peripheral Preferred Connection Parameters
--00000001-0000-3512-2118-0009af100700 //Unknown 1
--00000002-0000-3512-2118-0009af100700 //Unknown 2
--00000003-0000-3512-2118-0009af100700 //Configuration
--00000004-0000-3512-2118-0009af100700 //Unknown 4
--00000005-0000-3512-2118-0009af100700 //Activity data
--00000006-0000-3512-2118-0009af100700 //Battery Info
--00000007-0000-3512-2118-0009af100700 //Realtime steps
--00000008-0000-3512-2118-0009af100700 //User settings
--00000010-0000-3512-2118-0009af100700 //Device event
--0000000e-0000-3512-2118-0009af100700 //Weather
--00000020-0000-3512-2118-0009af100700
*/

class MiBandService : public BipService
{
    Q_OBJECT
public:
    MiBandService(QObject *parent);
    static const char* UUID_SERVICE_MIBAND;
    static const char* UUID_CHARACTERISTIC_MIBAND_CONFIGURATION;
    static const char* UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO;
    static const char* UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT;
    static const char* UUID_CHARACTERISTIC_MIBAND_NOTIFICATION;
    static const char* UUID_CHARACTERISTIC_MIBAND_CURRENT_TIME;
    static const char* UUID_CHARACTERISTIC_MIBAND_USER_SETTINGS;

    Q_PROPERTY(QString gpsVersion READ gpsVersion NOTIFY gpsVersionChanged())

    const char CHAR_RESPONSE = 0x10;
    const char CHAR_SUCCESS = 0x01;
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

    Q_INVOKABLE void requestGPSVersion();
    Q_INVOKABLE QString gpsVersion();
    Q_INVOKABLE void setCurrentTime();
    Q_INVOKABLE void setLanguage(int language);
    Q_INVOKABLE void setDateDisplay(int format);
    Q_INVOKABLE void setTimeFormat(int format);
    Q_INVOKABLE void setUserInfo();
    Q_INVOKABLE void setDistanceUnit();
    Q_INVOKABLE void setWearLocation();
    Q_INVOKABLE void setFitnessGoal();
    Q_INVOKABLE void setDisplayItems();
    Q_INVOKABLE void setDoNotDisturb();
    Q_INVOKABLE void setRotateWristToSwitchInfo(bool enable);
    Q_INVOKABLE void setActivateDisplayOnLiftWrist();
    Q_INVOKABLE void setDisplayCaller();
    Q_INVOKABLE void setGoalNotification(bool enable);
    Q_INVOKABLE void setInactivityWarnings();
    Q_INVOKABLE void setHeartrateSleepSupport();

    Q_SIGNAL void gpsVersionChanged();
    Q_SIGNAL void declineCall();
    Q_SIGNAL void ignoreCall();

private:
    Q_SLOT void characteristicRead(const QLowEnergyCharacteristic &c, const QByteArray &value);
    Q_SLOT void characteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
//    Q_SLOT void serviceReady(bool r);

    void setGPSVersion(const QString& v);

    QString m_gpsVersion;

    SettingsManager m_settings;
};

#endif // MIBANDSERVICE_H
