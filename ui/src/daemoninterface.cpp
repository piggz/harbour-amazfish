#include "daemoninterface.h"

#include <QDBusReply>
#include <QStandardPaths>

#include <KDb3/KDbDriverManager>

DaemonInterface::DaemonInterface(QObject *parent)
    : QObject(parent)
    , m_serviceWatcher(new QDBusServiceWatcher(
                           QStringLiteral(SERVICE_NAME), QDBusConnection::sessionBus(),
                           QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration))
{
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,   this, &DaemonInterface::connectDaemon);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &DaemonInterface::disconnectDaemon);

    connectDaemon();
    connectDatabase();
    m_dataSource.setConnection(dbConnection());
}

DaemonInterface::~DaemonInterface()
{
    iface->deleteLater();
}

QString DaemonInterface::activityToString(ActivityType type)
{
    if (type == NotMeasured) {
        return "NotMeasured";
    }
    if (type == Unknown) {
        return "Unknown";
    }
    if (type == Activity) {
        return "Activity";
    }
    if (type == LightSleep) {
        return "LightSleep";
    }
    if (type == DeepSleep) {
        return "DeepSleep";
    }
    if (type == NotWorn) {
        return "NotWorn";
    }
    if (type == Running) {
        return "Running";
    }
    if (type == Walking) {
        return "Walking";
    }
    if (type == Swimming) {
        return "Swimming";
    }
    if (type == Biking) {
        return "Biking";
    }
    if (type == Treadmill) {
        return "Treadmill";
    }

    return "Unknown";
}

void DaemonInterface::connectDaemon()
{
    qDebug() << "Connecting to daemon signals";

    if (iface) {
        iface->deleteLater();
    }

    iface = new QDBusInterface(QStringLiteral(SERVICE_NAME), QStringLiteral("/application"), QString(), QDBusConnection::sessionBus());

    if (!iface->isValid()) {
        iface->deleteLater();
        return;
    }

    connect(iface, SIGNAL(message(QString)), this, SIGNAL(message(QString)), Qt::UniqueConnection);
    connect(iface, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(operationRunningChanged()), this, SIGNAL(operationRunningChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(buttonPressed(int)), this, SIGNAL(buttonPressed(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(informationChanged(int, QString)), this, SIGNAL(informationChanged(int,QString)), Qt::UniqueConnection);

    //Property proxying
    connect(iface, SIGNAL(connectionStateChanged()), this, SLOT(changeConnectionState()), Qt::UniqueConnection);
    changeConnectionState();
}

void DaemonInterface::pair(const QString &name, QString address)
{
    qDebug(Q_FUNC_INFO);

    if (m_pairing) {
        return;
    }

    if (!iface->isValid()) {
        emit paired(name, address, tr("Unexpected error"));
        return;
    }

    m_pairing = true;
    emit pairingChanged();
    address.replace(QChar(':'), QChar('_')).prepend("/org/bluez/hci0/dev_");
    auto watcher = new QDBusPendingCallWatcher(iface->asyncCall(QStringLiteral("pair"), name, address));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, name, address]() {
        m_pairing = false;
        pairingChanged();
        emit paired(name, address, watcher->error().name());
        watcher->deleteLater();
    });
}

void DaemonInterface::changeConnectionState()
{
    qDebug(Q_FUNC_INFO);

    if (!iface->isValid()) {
        QString state(QStringLiteral("disconnected"));
        if (m_connectionState != state) {
            m_connectionState.swap(state);
            emit connectionStateChanged();
        }
        return;
    }

    auto watcher = new QDBusPendingCallWatcher(iface->asyncCall(QStringLiteral("connectionState")));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher]() {
        QDBusReply<QString> reply(watcher->reply());
        auto state = reply.value();
        if (m_connectionState != state) {
            m_connectionState.swap(state);
            emit connectionStateChanged();
        }
        watcher->deleteLater();
    });
}

void DaemonInterface::connectToDevice(const QString &address)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("connectToDevice"), address);
}

void DaemonInterface::disconnect()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("disconnect"));
}

bool DaemonInterface::supportsFeature(DaemonInterface::Feature f)
{
    if (!iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("supportsFeature"), f);
    return reply;
}

int DaemonInterface::supportedFeatures()
{
    if (!iface->isValid()) {
        return 0;
    }
    QDBusReply<int> reply = iface->call(QStringLiteral("supportedFeatures"));
    return reply;
}

DataSource *DaemonInterface::dataSource()
{
    return &m_dataSource;
}

KDbConnection *DaemonInterface::dbConnection()
{
    if (m_conn && m_conn->isDatabaseUsed()) {
        return m_conn;
    }
    return nullptr;
}

QString DaemonInterface::prepareFirmwareDownload(const QString &path)
{
    if (!iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call(QStringLiteral("prepareFirmwareDownload"), path);
    return reply;
}

bool DaemonInterface::startDownload()
{
    if (!iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("startDownload"));
    return reply;
}

void DaemonInterface::downloadSportsData()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("downloadSportsData");
}

void DaemonInterface::downloadActivityData()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("downloadActivityData"));
}

void DaemonInterface::refreshInformation()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("refreshInformation"));
}

QString DaemonInterface::information(DaemonInterface::Info i)
{
    if (!iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call(QStringLiteral("information"), i);
    return reply;
}

void DaemonInterface::sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("sendAlert"), sender, subject, message, allowDuplicate);
}

void DaemonInterface::incomingCall(const QString &caller)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("incomingCall"), caller);
}

void DaemonInterface::applyDeviceSetting(Settings s)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("applyDeviceSetting"), s);
}

void DaemonInterface::requestManualHeartrate()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("requestManualHeartrate"));
}

void DaemonInterface::triggerSendWeather()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("triggerSendWeather"));
}

bool DaemonInterface::operationRunning()
{
    if (!iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call(QStringLiteral("operationRunning"));
    return reply;
}

void DaemonInterface::connectDatabase()
{
    KDbDriverManager manager;

    const QStringList driverIds = manager.driverIds();
    if (driverIds.isEmpty()) {
        qWarning() << "No drivers found";
        return;
    }
    qDebug() << "DRIVERS: " << driverIds;

    if (manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    //get driver
    m_dbDriver = manager.driver(QStringLiteral("org.kde.kdb.sqlite"));
    if (!m_dbDriver || manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    m_connData.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation).append("/amazfish.kexi"));

    qDebug() << "Database is: " << m_connData.databaseName();

    m_conn = m_dbDriver->createConnection(m_connData);

    if (!m_conn || m_dbDriver->result().isError()) {
        qDebug() << m_dbDriver->result();
        return;
    }    
    qDebug() << "KDbConnection object created.";

    if (!m_conn->connect()) {
        qDebug() << m_conn->result();
        return;
    }
    qDebug() << "KDbConnection::connect() OK.";

    if (!m_conn->databaseExists(m_connData.databaseName())) {
        if (!m_conn->createDatabase(m_connData.databaseName())) {
            qDebug() << m_conn->result();
            return;
        }
        qDebug() << "DB" << m_connData.databaseName() << "created";
    }

    if (!m_conn->useDatabase()) {
        qDebug() << m_conn->result();
        return;
    }
}

void DaemonInterface::updateCalendar()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("updateCalendar"));
}

void DaemonInterface::reloadCities()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call(QStringLiteral("reloadCities"));
}
