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
#include "amazfish.h"

#define SERVICE_NAME "uk.co.piggz.amazfish"

class DaemonInterface : public QObject
{
    Q_OBJECT

    //Device Interface
    Q_PROPERTY(QString connectionState MEMBER m_connectionState NOTIFY connectionStateChanged)
    Q_PROPERTY(int connectionStateChangedCount MEMBER m_connectionStateChangedCount NOTIFY connectionStateChangedCountChanged)
    Q_PROPERTY(bool operationRunning READ operationRunning NOTIFY operationRunningChanged)

public:
    explicit DaemonInterface(QObject *parent = nullptr);
    ~DaemonInterface();

    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void unpair();
    Q_INVOKABLE bool supportsFeature(Amazfish::Feature f);
    Q_INVOKABLE int supportedFeatures();
    Q_INVOKABLE int supportedDataTypes();

    Q_INVOKABLE DataSource *dataSource();
    KDbConnection *dbConnection();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE bool startDownload();
    Q_INVOKABLE void downloadSportsData();
    Q_INVOKABLE void downloadActivityData();
    Q_INVOKABLE void fetchData(int dataTypes);
    Q_INVOKABLE void refreshInformation();
    Q_INVOKABLE QString information(Amazfish::Info i);
    Q_INVOKABLE void sendAlert(const int notificationId, const QString &sender, const QString &subject, const QString &message, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void applyDeviceSetting(Amazfish::Settings s);
    Q_INVOKABLE void requestManualHeartrate();
    Q_INVOKABLE void triggerSendWeather();
    Q_INVOKABLE void updateCalendar();
    Q_INVOKABLE void reloadCities();
    Q_INVOKABLE void enableFeature(Amazfish::Feature feature);
    Q_INVOKABLE void fetchLogs();
    Q_INVOKABLE void requestScreenshot();
    Q_INVOKABLE QStringList supportedDisplayItems();
    Q_INVOKABLE void immediateAlert(int level);

public slots:
    void pair(const QString &name, const QString &deviceType, const QString &address);


signals:
    void paired(const QString &name, const QString &address, const QString &error);
    void pairingChanged();
    void message(const QString &text);
    void downloadProgress(int percent);
    void operationRunningChanged();
    void buttonPressed(int presses);
    void informationChanged(int infoKey, const QString& infoValue);
    void connectionStateChanged();
    void connectionStateChangedCountChanged();

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
    int m_connectionStateChangedCount;

    bool operationRunning();

};

#endif // DAEMONINTERFACE_H
