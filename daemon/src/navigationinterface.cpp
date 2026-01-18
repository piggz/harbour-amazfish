#include "navigationinterface.h"
#include <QDebug>
#include <QDBusReply>
#include <unistd.h>

NavigationInterface::NavigationInterface(QObject *parent) : QObject(parent)
  , m_serviceWatcher(new QDBusServiceWatcher(
                         QStringLiteral(SERVICE_NAME_MAPS), QDBusConnection::sessionBus(),
                         QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration))
{
  QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,   this, &NavigationInterface::connectDaemon);
  QObject::connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered, this, &NavigationInterface::disconnectDaemon);

  connectDaemon();
}

void NavigationInterface::connectDaemon()
{

    if (iface) {
        iface->deleteLater();
    }

    iface = new QDBusInterface(QStringLiteral(SERVICE_NAME_MAPS), QStringLiteral(OBJECT_PATH), QStringLiteral(INTERFACE_NAME), QDBusConnection::sessionBus());

    if (!iface->isValid()) {
        qDebug() << Q_FUNC_INFO << "Interface is not valid";

        iface->deleteLater();
        iface = nullptr;
        return;
    }
    qDebug() << Q_FUNC_INFO << iface;

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
    if (!iface || !iface->isValid()) {
        qWarning() << Q_FUNC_INFO << iface;
        return;
    }
    bool reply = iface->property("running").toBool();
    qDebug() << Q_FUNC_INFO << reply;

    if (m_running != reply) {
        m_running = reply;
        emit runningChanged(m_running);
    }
}

void NavigationInterface::slot_iconChanged()
{

    if (!iface || !iface->isValid()) {
        qWarning() << Q_FUNC_INFO << iface;
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("icon").toString();
    qDebug() << Q_FUNC_INFO << reply;

    if (m_icon != reply) {
        m_icon = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}

void NavigationInterface::slot_manDistChanged()
{
    if (!iface || !iface->isValid()) {
        qWarning() << Q_FUNC_INFO << iface;
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("manDist").toString();
    qDebug() << Q_FUNC_INFO << reply;

    if (m_manDist != reply) {
        m_manDist = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}

void NavigationInterface::slot_narrativeChanged()
{
    if (!iface || !iface->isValid()) {
        qWarning() << Q_FUNC_INFO << iface;
        return;
    }
    m_progress = iface->property("progress").toInt();
    QString reply = iface->property("narrative").toString();
    qDebug() << Q_FUNC_INFO << reply;
    if (m_narrative != reply) {
        m_narrative = reply;
        emit navigationChanged(m_icon, m_narrative, m_manDist, m_progress);
    }
}
