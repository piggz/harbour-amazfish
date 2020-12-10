#include "navigationinterface.h"
#include <QDebug>
#include <QDBusReply>
#include <unistd.h>

NavigationInterface::NavigationInterface(QObject *parent) : QObject(parent)
  , m_serviceWatcher(new QDBusServiceWatcher(
                         QStringLiteral(SERVICE_NAME), QDBusConnection::sessionBus(),
                         QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration))
{
  QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,   this, &NavigationInterface::connectDaemon);
  QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &NavigationInterface::disconnectDaemon);

  connectDaemon();
}

void NavigationInterface::connectDaemon()
{
    qDebug() << Q_FUNC_INFO;
    sleep(3);

    if (iface) {
        iface->deleteLater();
    }

    iface = new QDBusInterface(QStringLiteral(SERVICE_NAME), QStringLiteral(OBJECT_PATH), QStringLiteral(INTERFACE_NAME), QDBusConnection::sessionBus());

    if (!iface->isValid()) {
        qDebug() << "Interface is not valid";

        iface->deleteLater();
        iface = nullptr;
        return;
    }

    connect(iface, SIGNAL(runningChanged()), this, SLOT(slot_runningChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(narrativeChanged()), this, SLOT(slot_narrativeChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(iconChanged()), this, SLOT(slot_iconChanged()), Qt::UniqueConnection);
    connect(iface, SIGNAL(manDistChanged()), this, SLOT(slot_manDistChanged()), Qt::UniqueConnection);

}

void NavigationInterface::disconnectDaemon()
{

}

void NavigationInterface::slot_runningChanged()
{
    qDebug() << Q_FUNC_INFO;

    if (!iface || !iface->isValid()) {
        return;
    }
    bool reply = iface->property("running").toBool();
    qDebug() << reply;

    if (m_running != reply) {
        m_running = reply;
        emit runningChanged(m_running);
    }
}

void NavigationInterface::slot_iconChanged()
{
    qDebug() << Q_FUNC_INFO;

    if (!iface || !iface->isValid()) {
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("icon").toString();
    qDebug() << reply;

    if (m_icon != reply) {
        m_icon = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}

void NavigationInterface::slot_manDistChanged()
{
    qDebug() << Q_FUNC_INFO;

    if (!iface || !iface->isValid()) {
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("manDist").toString();
    qDebug() << reply;

    if (m_manDist != reply) {
        m_manDist = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}

void NavigationInterface::slot_narrativeChanged()
{
    qDebug() << Q_FUNC_INFO;

    if (!iface || !iface->isValid()) {
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("narrative").toString();
    qDebug() << reply;
    if (m_narrative != reply) {
        m_narrative = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}
