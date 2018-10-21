
#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include "qble/qbledevice.h"
#include "weather/currentweather.h"

class AbstractDevice : public QBLEDevice
{
    Q_OBJECT
    Q_ENUMS(Feature)
    Q_ENUMS(Info)
    
    public:
    enum Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER,
        FEATURE_ACTIVITY,
        FEATURE_STEPS,
        FEATURE_ALARMS,
        FEATURE_ALERT,
        FEATURE_NOTIFIATION
    };
    
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
    
    explicit AbstractDevice(QObject *parent = 0);
    
    virtual bool supportsFeature(Feature f) = 0;
    virtual QString deviceType() = 0;
    virtual QString deviceName() = 0;
    virtual bool operationRunning() = 0;
    virtual QString connectionState() const = 0;
    
    virtual QString prepareFirmwareDownload(const QString &path);
    virtual void startDownload();
    virtual void downloadSportsData();
    virtual void sendWeather(CurrentWeather *weather);
    virtual void refreshInformation();
    virtual QString information(Info i);
    virtual void applyDeviceSettings(Settings s);
    
    //signals    
    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void connectionStateChanged();

};

#endif