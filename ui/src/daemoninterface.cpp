#include "daemoninterface.h"

#include <QDBusReply>

DaemonInterface::DaemonInterface(QObject *parent) : QObject(parent)
{
    iface = new QDBusInterface(SERVICE_NAME, "/", "", QDBusConnection::sessionBus());

    connect(iface, SIGNAL(message(QString)), this, SIGNAL(message(QString)));
    connect(iface, SIGNAL(downloadProgress(int)), this, SIGNAL(downloadProgress(int)));
    connect(iface, SIGNAL(operationRunningChanged()), this, SIGNAL(operationRunningChanged()));
    connect(iface, SIGNAL(buttonPressed(int)), this, SIGNAL(buttonPressed(int)));
    connect(iface, SIGNAL(informationChanged(int, QString)), this, SIGNAL(informationChanged(int,QString)));
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

QString DaemonInterface::pair(const QString &name, const QString &address)
{
    QDBusReply<QString> reply = iface->call("pair", name, address);
    return reply;
}

void DaemonInterface::connectToDevice(const QString &address)
{
    iface->call("connectToDevice", address);
}

void DaemonInterface::disconnect()
{
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
    QDBusReply<QString> reply = iface->call("prepareFirmwareDownload", path);
    return reply;
}

void DaemonInterface::startDownload()
{
    iface->call("startDownload");
}

void DaemonInterface::downloadSportsData()
{
    iface->call("downloadSportsData");
}

void DaemonInterface::downloadActivityData()
{
    iface->call("downloadActivityData");
}

void DaemonInterface::sendWeather(CurrentWeather *weather)
{
    //iface->call("downloadActivityData");
}

void DaemonInterface::refreshInformation()
{
    iface->call("refreshInformation");
}

QString DaemonInterface::information(DaemonInterface::Info i)
{
    QDBusReply<QString> reply = iface->call("information", i);
    return reply;
}

void DaemonInterface::sendAlert(const QString &sender, const QString &subject, const QString &message, bool allowDuplicate)
{
    iface->call("sendAlert", sender, subject, message, allowDuplicate);
}

void DaemonInterface::incomingCall(const QString &caller)
{
    iface->call("incomingCall", caller);
}

void DaemonInterface::applyDeviceSetting(Settings s)
{
    iface->call("applyDeviceSetting", s);
}

void DaemonInterface::requestManualHeartrate()
{
    iface->call("requestManualHeartrate");
}

QString DaemonInterface::connectionState() const
{
    QDBusReply<QString> reply = iface->call("connectionState");
    return reply;
}

bool DaemonInterface::operationRunning()
{
    QDBusReply<bool> reply = iface->call("operationRunning");
    return reply;
}
