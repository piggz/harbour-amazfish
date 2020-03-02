
#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include "qble/qbledevice.h"
#include "weather/currentweather.h"
#include "abstractfirmwareinfo.h"

class AbstractDevice : public QBLEDevice
{
    Q_OBJECT

public:
    enum Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER = 2,
        FEATURE_ACTIVITY = 4,
        FEATURE_STEPS = 8,
        FEATURE_ALARMS = 16,
        FEATURE_ALERT = 32,
        FEATURE_NOTIFICATION = 64,
        FEATURE_EVENT_REMINDER = 128
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
        SETTING_DEVICE_UNIT,
        SETTING_DISCONNECT_NOTIFICATION
    };
    Q_ENUM(Settings)

    explicit AbstractDevice(const QString &pairedName, QObject *parent = 0);
    
    virtual QString pair() override;
    virtual void pairAsync() override;

    virtual void connectToDevice() override;
    virtual void disconnectFromDevice() override;
    virtual QString connectionState() const;

    bool supportsFeature(Feature f);
    virtual int supportedFeatures() = 0;

    virtual QString deviceType() = 0;
    QString deviceName();
    virtual bool operationRunning() = 0;
    virtual void abortOperations();

    //Firmware handling
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) = 0; //Caller owns the pointer and should delete it
    virtual void prepareFirmwareDownload(const AbstractFirmwareInfo* info);
    virtual void startDownload();

    virtual void downloadSportsData();
    virtual void sendWeather(CurrentWeather *weather);
    virtual void refreshInformation();
    virtual QString information(Info i);
    virtual void applyDeviceSetting(Settings s);
    virtual void rebootWatch();
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) = 0;
    virtual void incomingCall(const QString &caller) = 0;
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event);

    //signals    
    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void connectionStateChanged();
    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString& val);

protected:
    bool m_needsAuth = false;
    bool m_pairing = false;
    bool m_ready = false;
    bool m_hasInitialised = false;
    bool m_autoreconnect = true;
    QString m_connectionState;
    QTimer *m_reconnectTimer;

    void setConnectionState(const QString &state);

private:
    void reconnectionTimer();
    QString m_pairedName;
};

#endif
