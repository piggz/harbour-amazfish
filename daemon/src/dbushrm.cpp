#include "dbushrm.h"
#include <QDebug>

DBusHRM::DBusHRM(QObject *parent) : QObject(parent)
{
    qDebug() << Q_FUNC_INFO;

    if (!QDBusConnection::sessionBus().registerService("org.sailfishos.heartrate")) {
        qDebug() << Q_FUNC_INFO << "Unable to register service:" << QDBusConnection::sessionBus().lastError().message();
    } else {
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    }
}

void DBusHRM::setHRMService(HRMService *hrm)
{
    qDebug() << Q_FUNC_INFO;
    m_hrm = hrm;
}

void DBusHRM::setDevice(AbstractDevice *dev)
{
    qDebug() << Q_FUNC_INFO;
    m_device = dev;
}

void DBusHRM::start()
{
    qDebug() << Q_FUNC_INFO;
    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(true);
    }
}

void DBusHRM::stop()
{
    qDebug() << Q_FUNC_INFO;
    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(false);
    }
}

int DBusHRM::heartRate()
{
    qDebug() << Q_FUNC_INFO << m_hrm->heartRate();

    if (m_hrm) {
        m_hrm->keepRealtimeHRMMeasurementAlive();
        return m_hrm->heartRate();
    }
    return 0;
}

int DBusHRM::batteryLevel()
{
    if (m_device) {
        return m_device->information(AbstractDevice::INFO_BATTERY).toInt();
    }
    return 0;
}
