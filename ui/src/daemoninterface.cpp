#include "daemoninterface.h"

#include <QDBusReply>
#include <QStandardPaths>

#include <KDb3/KDbDriverManager>

DaemonInterface::DaemonInterface(QObject *parent) : QObject(parent)
{
    iface = new QDBusInterface(SERVICE_NAME, "/application", "", QDBusConnection::sessionBus());

    m_serviceWatcher = new QDBusServiceWatcher(SERVICE_NAME, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DaemonInterface::connectDaemon);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &DaemonInterface::disconnectDaemon);

    if (iface->isValid()){
        connectDaemon();
    }

    connectDatabase();
    m_dataSource.setConnection(dbConnection());
}

DaemonInterface::~DaemonInterface()
{
    delete iface;
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
        delete iface;
    }

    iface = new QDBusInterface(SERVICE_NAME, "/application", "", QDBusConnection::sessionBus());

    connect(iface, SIGNAL(message(QString)), this, SIGNAL(message(QString)), Qt::UniqueConnection);
    connect(iface, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(operationRunningChanged()), this, SIGNAL(operationRunningChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(buttonPressed(int)), this, SIGNAL(buttonPressed(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(informationChanged(int, QString)), this, SIGNAL(informationChanged(int,QString)), Qt::UniqueConnection);

    //Property proxying
    connect(iface, SIGNAL(connectionStateChanged()), this, SLOT(slot_connectionStateChanged()), Qt::UniqueConnection);
    slot_connectionStateChanged();
}

void DaemonInterface::disconnectDaemon()
{
    slot_connectionStateChanged();
}

QString DaemonInterface::pair(const QString &name, const QString &address)
{
    if (!iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call("pair", name, address);
    return reply;
}

void DaemonInterface::connectToDevice(const QString &address)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("connectToDevice", address);
}

void DaemonInterface::disconnect()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("disconnect");
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
    QDBusReply<QString> reply = iface->call("prepareFirmwareDownload", path);
    return reply;
}

bool DaemonInterface::startDownload()
{
    if (!iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call("startDownload");
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
    iface->call("downloadActivityData");
}

void DaemonInterface::refreshInformation()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("refreshInformation");
}

QString DaemonInterface::information(DaemonInterface::Info i)
{
    if (!iface->isValid()) {
        return QString();
    }
    QDBusReply<QString> reply = iface->call("information", i);
    return reply;
}

void DaemonInterface::sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("sendAlert", sender, subject, message, allowDuplicate);
}

void DaemonInterface::incomingCall(const QString &caller)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("incomingCall", caller);
}

void DaemonInterface::applyDeviceSetting(Settings s)
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("applyDeviceSetting", s);
}

void DaemonInterface::requestManualHeartrate()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("requestManualHeartrate");
}

void DaemonInterface::triggerSendWeather()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("triggerSendWeather");
}

void DaemonInterface::slot_connectionStateChanged()
{
    if (!iface->isValid()) {
        m_connectionState = "disconnected";
    } else {
        QDBusReply<QString> reply = iface->call("connectionState");
        m_connectionState = reply;
    }

    emit connectionStateChanged();
}

QString DaemonInterface::connectionState() const
{
    return m_connectionState;
}

bool DaemonInterface::operationRunning()
{
    if (!iface->isValid()) {
        return false;
    }
    QDBusReply<bool> reply = iface->call("operationRunning");
    return reply;
}

void DaemonInterface::connectDatabase()
{
    KDbDriverManager manager;
    const QStringList driverIds = manager.driverIds();
    qDebug() << "DRIVERS: " << driverIds;
    if (driverIds.isEmpty()) {
        qWarning() << "No drivers found";
        return;
    }
    if (manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    //get driver
    m_dbDriver = manager.driver("org.kde.kdb.sqlite");
    if (!m_dbDriver || manager.result().isError()) {
        qDebug() << manager.result();
        return;
    }

    m_connData.setDatabaseName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+ "/amazfish.kexi");

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
    iface->call("updateCalendar");
}
