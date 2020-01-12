
#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>
#include <QtSql/QtSql>
#include <QQueue>
#include <QTimer>

#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>

#include "qble/bluezadapter.h"

#include "abstractdevice.h"
#include "notificationslistener.h"
#include "voicecallhandler.h"
#include "voicecallmanager.h"
#include "settingsmanager.h"
#include "dbushrm.h"
#include "weather/citymanager.h"
#include "weather/currentweather.h"

class AlertNotificationService;
class DeviceInfoService;
class HRMService;
class MiBand2Service;
class MiBandService;
class BipFirmwareService;

#define SERVICE_NAME "uk.co.piggz.amazfish"

class DeviceInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME)
public:
    DeviceInterface();
    ~DeviceInterface();

    //Copied from Abstract device due to QML
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
    Q_ENUM(Info);

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
    Q_ENUM(Settings);

    struct WatchNotification
    {
        QString appName;
        QString summary;
        QString body;
    };

    void registerDBus();

    Q_INVOKABLE QString pair(const QString &name, const QString &address);
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE QString connectionState() const;
    Q_INVOKABLE bool operationRunning();

    KDbConnection *dbConnection();

    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void informationChanged(int infoKey, const QString& infoValue);
    Q_SIGNAL void connectionStateChanged();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE void startDownload();
    Q_INVOKABLE void downloadSportsData();
    Q_INVOKABLE void downloadActivityData();
    Q_INVOKABLE void refreshInformation();
    Q_INVOKABLE QString information(int i);
    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void applyDeviceSetting(int s);
    Q_INVOKABLE void requestManualHeartrate();
    Q_INVOKABLE void triggerSendWeather();

private:
    QString m_deviceAddress;
    QString m_deviceName;
    bool m_dbusRegistered = false;
    int m_lastBatteryLevel = 0;
    int m_lastAlertHash = 0;

    BluezAdapter m_adapter;
    AbstractDevice *m_device = nullptr;
    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    SettingsManager m_settings;
    DBusHRM *m_dbusHRM = nullptr;

    QTimer *m_refreshTimer = nullptr;
    Q_SLOT void onRefreshTimer();

    void createSettings();
    void updateServiceController();

    //TODO Minimise use of these funcitons
    MiBandService *miBandService() const;
    AlertNotificationService *alertNotificationService() const;
    HRMService *hrmService() const;
    BipFirmwareService *firmwareService() const;
    
    Q_SLOT void notificationReceived(const QString &appName, const QString &summary, const QString &body);
    Q_SLOT void onActiveVoiceCallChanged();
    Q_SLOT void onActiveVoiceCallStatusChanged();
    Q_SLOT void onConnectionStateChanged();
    Q_SLOT void slot_informationChanged(AbstractDevice::Info infokey, const QString &infovalue);
    void sendBufferedNotifications();

    QQueue<WatchNotification> m_notificationBuffer;

    //Database
    KDbDriver *m_dbDriver = nullptr;
    KDbConnectionData m_connData;
    KDbConnection *m_conn = nullptr;
    void setupDatabase();
    void createTables();

    //Weather
    CityManager m_cityManager;
    CurrentWeather m_currentWeather;
    void sendWeather(CurrentWeather *weather);
    Q_SLOT void onCitiesChanged();
    Q_SLOT void onWeatherReady();

};

#endif // BIPINTERFACE_H
