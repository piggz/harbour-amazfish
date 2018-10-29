
#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>
#include <QtSql/QtSql>
#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>

#include "qble/bluezadapter.h"

#include "abstractdevice.h"
#include "notificationslistener.h"
#include "voicecallhandler.h"
#include "voicecallmanager.h"
#include "settingsmanager.h"
#include "datasource.h"
#include "dbushrm.h"

class AlertNotificationService;
class DeviceInfoService;
class HRMService;
class MiBand2Service;
class MiBandService;
class BipFirmwareService;

class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    DeviceInterface();

    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(bool operationRunning READ operationRunning NOTIFY operationRunningChanged)

    Q_INVOKABLE QString pair(const QString &address);
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();

    bool ready() const;
    QString connectionState() const;
    
    Q_INVOKABLE DataSource *dataSource();
    KDbConnection *dbConnection();

    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void informationChanged(int infoKey, const QString& infoValue);

    Q_INVOKABLE bool operationRunning();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE void startDownload();
    Q_INVOKABLE void downloadSportsData();
    Q_INVOKABLE void sendWeather(CurrentWeather *weather);
    Q_INVOKABLE void refreshInformation();
    Q_INVOKABLE QString information(AbstractDevice::Info i);
    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void applyDeviceSettings(AbstractDevice::Settings s);


private:
    QString m_deviceAddress;
    QString m_deviceName;

    BluezAdapter m_adapter;
    AbstractDevice *m_bipDevice = nullptr;
    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    SettingsManager m_settings;
    DataSource m_dataSource;
    DBusHRM *m_dbusHRM = nullptr;

    void createSettings();
    void updateServiceController();
    
    DeviceInfoService *infoService() const;
    MiBandService *miBandService() const;
    MiBand2Service *miBand2Service() const;
    AlertNotificationService *alertNotificationService() const;
    HRMService *hrmService() const;
    BipFirmwareService *firmwareService() const;
    
    Q_SLOT void notificationReceived(const QString &appName, const QString &summary, const QString &body);
    Q_SLOT void onActiveVoiceCallChanged();
    Q_SLOT void onActiveVoiceCallStatusChanged();
    Q_SLOT void onConnectionStateChanged();
    Q_SLOT void slot_informationChanged(AbstractDevice::Info infokey, const QString &infovalue);

    //Database
    KDbDriver *m_dbDriver = nullptr;
    KDbConnectionData m_connData;
    KDbConnection *m_conn = nullptr;
    void setupDatabase();
    void createTables();
Q_SIGNALS:
    void readyChanged();
    void connectionStateChanged();
};

#endif // BIPINTERFACE_H
