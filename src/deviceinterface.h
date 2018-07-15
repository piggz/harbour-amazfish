#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>
#include <QtSql/QtSql>
#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>

#include "qble/bluezadapter.h"

#include "bipdevice.h"
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

    Q_INVOKABLE DeviceInfoService *infoService() const;
    Q_INVOKABLE MiBandService *miBandService() const;
    Q_INVOKABLE MiBand2Service *miBand2Service() const;
    Q_INVOKABLE AlertNotificationService *alertNotificationService() const;
    Q_INVOKABLE HRMService *hrmService() const;
    Q_INVOKABLE BipFirmwareService *firmwareService() const;
    
    Q_INVOKABLE DataSource *dataSource();

    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();

    Q_INVOKABLE bool operationRunning();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE void startDownload();
    Q_INVOKABLE void downloadSportsData();


    KDbConnection *dbConnection();

private:
    QString m_deviceAddress;
    QString m_deviceName;

    BluezAdapter m_adapter;
    BipDevice *m_bipDevice = nullptr;
    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    SettingsManager m_settings;
    DataSource m_dataSource;
    DBusHRM *m_dbusHRM = nullptr;


    void createSettings();

    void updateServiceController();

    Q_SLOT void notificationReceived(const QString &appName, const QString &summary, const QString &body);
    Q_SLOT void onActiveVoiceCallChanged();
    Q_SLOT void onActiveVoiceCallStatusChanged();
    Q_SLOT void onConnectionStateChanged();

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
