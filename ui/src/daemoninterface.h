#ifndef DAEMONINTERFACE_H
#define DAEMONINTERFACE_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusInterface>
#include <QDBusServiceWatcher>

#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>

#include "datasource.h"
#include "weather/currentweather.h"

#define SERVICE_NAME "uk.co.piggz.amazfish"

class DaemonInterface : public QObject
{
    Q_OBJECT
public:
    explicit DaemonInterface(QObject *parent = nullptr);
    ~DaemonInterface();

    enum ActivityType {
        NotMeasured = -1,
        Unknown = 0,
        Activity = 1,
        LightSleep = 2,
        DeepSleep = 4,
        NotWorn = 8,
        Running = 16,
        Walking = 32,
        Swimming = 64,
        Biking = 128,
        Treadmill = 256
    };
    Q_ENUM(ActivityType)

    enum Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER,
        FEATURE_ACTIVITY,
        FEATURE_STEPS,
        FEATURE_ALARMS,
        FEATURE_ALERT,
        FEATURE_NOTIFIATION
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
        SETTING_DEVICE_UNIT
    };
    Q_ENUM(Settings)

    static QString activityToString(ActivityType type);

    //Device Interface
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(bool operationRunning READ operationRunning NOTIFY operationRunningChanged)

    Q_INVOKABLE QString pair(const QString &name, const QString &address);
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();

    Q_INVOKABLE DataSource *dataSource();
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
    Q_INVOKABLE QString information(Info i);
    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void applyDeviceSetting(Settings s);
    Q_INVOKABLE void requestManualHeartrate();
    Q_INVOKABLE void triggerSendWeather();

private:
    QDBusInterface *iface = nullptr;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    DataSource m_dataSource;

    //Database
    KDbDriver *m_dbDriver = nullptr;
    KDbConnectionData m_connData;
    KDbConnection *m_conn = nullptr;

    void connectDaemon();
    void disconnectDaemon();

    void connectDatabase();

    QString m_connectionState;
    QString connectionState() const;
    Q_SLOT void slot_connectionStateChanged();

    bool operationRunning();

};

#endif // DAEMONINTERFACE_H
