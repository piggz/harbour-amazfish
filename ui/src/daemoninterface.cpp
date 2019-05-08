#include "daemoninterface.h"

#include <QDBusReply>

DaemonInterface::DaemonInterface(QObject *parent) : QObject(parent)
{
    iface = new QDBusInterface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());

    m_serviceWatcher = new QDBusServiceWatcher(SERVICE_NAME, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForRegistration);
    QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &DaemonInterface::connectDaemon);

    if (iface->isValid()){
        connectDaemon();
    }
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

    connect(iface, SIGNAL(message(QString)), this, SIGNAL(message(QString)), Qt::UniqueConnection);
    connect(iface, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(operationRunningChanged()), this, SIGNAL(operationRunningChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(buttonPressed(int)), this, SIGNAL(buttonPressed(int)), Qt::UniqueConnection);
    connect(iface, SIGNAL(informationChanged(int, QString)), this, SIGNAL(informationChanged(int,QString)), Qt::UniqueConnection);

    //Property proxying
    connect(iface, SIGNAL(connectionStateChanged()), this, SLOT(slot_connectionStateChanged()), Qt::UniqueConnection);
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
    return nullptr;
}

KDbConnection *DaemonInterface::dbConnection()
{
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

void DaemonInterface::startDownload()
{
    if (!iface->isValid()) {
        return;
    }
    iface->call("startDownload");
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

void DaemonInterface::sendWeather(CurrentWeather *weather)
{
    //iface->call("downloadActivityData");
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

void DaemonInterface::slot_connectionStateChanged()
{
    if (!iface->isValid()) {
        return;
    }
    QDBusReply<QString> reply = iface->call("connectionState");
    m_connectionState = reply;

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
