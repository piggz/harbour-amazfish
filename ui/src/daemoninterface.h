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

#define SERVICE_NAME "uk.co.piggz.amazfish"

class DaemonInterface : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool pairing MEMBER m_pairing NOTIFY pairingChanged)

    //Device Interface
    Q_PROPERTY(QString connectionState MEMBER m_connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(bool operationRunning READ operationRunning NOTIFY operationRunningChanged)

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
        FEATURE_WEATHER = 2,
        FEATURE_ACTIVITY = 4,
        FEATURE_STEPS = 8,
        FEATURE_ALARMS = 16,
        FEATURE_ALERT = 32,
        FEATURE_NOTIFIATION = 64,
        FEATURE_EVENT_REMINDER = 128,
        FEATURE_MUSIC_CONTROL = 256
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
        INFO_MANUFACTURER
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

    static QString activityToString(ActivityType type);

    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE bool supportsFeature(Feature f);
    Q_INVOKABLE int supportedFeatures();

    Q_INVOKABLE DataSource *dataSource();
    KDbConnection *dbConnection();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE bool startDownload();
    Q_INVOKABLE void downloadSportsData();
    Q_INVOKABLE void downloadActivityData();
    Q_INVOKABLE void refreshInformation();
    Q_INVOKABLE QString information(Info i);
    Q_INVOKABLE void sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void applyDeviceSetting(Settings s);
    Q_INVOKABLE void requestManualHeartrate();
    Q_INVOKABLE void triggerSendWeather();
    Q_INVOKABLE void updateCalendar();
    Q_INVOKABLE void reloadCities();
    Q_INVOKABLE void enableFeature(Feature feature);

public slots:
    void pair(const QString &name, QString address);

signals:
    void paired(const QString &name, const QString &address, const QString &error);
    void pairingChanged();
    void message(const QString &text);
    void downloadProgress(int percent);
    void operationRunningChanged();
    void buttonPressed(int presses);
    void informationChanged(int infoKey, const QString& infoValue);
    void connectionStateChanged();

private slots:
    void changeConnectionState();

private:
    QDBusInterface *iface = nullptr;
    QDBusServiceWatcher *m_serviceWatcher = nullptr;
    DataSource m_dataSource;
    bool m_pairing = false;

    //Database
    KDbDriver *m_dbDriver = nullptr;
    KDbConnectionData m_connData;
    KDbConnection *m_conn = nullptr;

    void connectDaemon();
    void disconnectDaemon()
    { changeConnectionState(); }

    void connectDatabase();

    QString m_connectionState;

    bool operationRunning();

};

#endif // DAEMONINTERFACE_H
