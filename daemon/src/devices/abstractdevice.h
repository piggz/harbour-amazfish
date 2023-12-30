
#ifndef ABSTRACTDEVICE_H
#define ABSTRACTDEVICE_H

#include "qble/qbledevice.h"
#include "weather/currentweather.h"
#include "abstractfirmwareinfo.h"

#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>
#include <KDb3/KDbTransactionGuard>

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
        FEATURE_EVENT_REMINDER = 64,
        FEATURE_MUSIC_CONTROL = 128,
        FEATURE_BUTTON_ACTION = 256
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
        INFO_HEARTRATE,
        INFO_MODEL,
        INFO_FW_REVISION,
        INFO_MANUFACTURER,
        INFO_IMMEDIATE_ALERT
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

    enum Event {
        EVENT_MUSIC_STOP,
        EVENT_MUSIC_PLAY,
        EVENT_MUSIC_PAUSE,
        EVENT_MUSIC_NEXT,
        EVENT_MUSIC_PREV,
        EVENT_MUSIC_VOLUP,
        EVENT_MUSIC_VOLDOWN,
        EVENT_APP_MUSIC,
        EVENT_DECLINE_CALL,
        EVENT_ANSWER_CALL,
        EVENT_IGNORE_CALL,
        EVENT_FIND_PHONE,
        EVENT_CANCEL_FIND_PHONE
    };
    Q_ENUM(Event)

    explicit AbstractDevice(const QString &pairedName, QObject *parent = nullptr);
    
    virtual void pair() override;

    virtual void connectToDevice() override;
    virtual void disconnectFromDevice() override;
    virtual QString connectionState() const;

    bool supportsFeature(Feature f) const;
    virtual int supportedFeatures() const = 0;

    virtual void setDatabase(KDbConnection *conn);

    virtual QString deviceType() const = 0;
    QString deviceName() const;
    virtual void abortOperations();

    //Firmware handling
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) = 0; //Caller owns the pointer and should delete it
    virtual void prepareFirmwareDownload(const AbstractFirmwareInfo* info);
    virtual void startDownload();

    virtual void downloadSportsData();
    virtual void downloadActivityData();
    virtual void fetchLogs();
    virtual void sendWeather(CurrentWeather *weather);
    virtual void refreshInformation();
    virtual QString information(Info i) const;
    virtual void applyDeviceSetting(Settings s);
    virtual void rebootWatch();
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) = 0;
    virtual void incomingCall(const QString &caller) = 0;
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event);
    virtual void enableFeature(AbstractDevice::Feature feature);
    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0);
    virtual void navigationRunning(bool running);
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress);
    virtual QStringList supportedDisplayItems() const;

    //signals    
    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void connectionStateChanged();
    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString& val);
    Q_SIGNAL void deviceEvent(Event event);

protected:
    bool m_needsAuth = false;
    bool m_pairing = false;
    bool m_ready = false;
    bool m_hasInitialised = false;
    bool m_autoreconnect = true;
    QString m_connectionState;
    QTimer *m_reconnectTimer;

    void setConnectionState(const QString &state);
    KDbConnection *m_conn = nullptr;

private:
    void reconnectionTimer();
    void devicePairFinished(const QString& status);
    QString m_pairedName;

};

#endif
