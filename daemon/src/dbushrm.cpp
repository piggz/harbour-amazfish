#include "dbushrm.h"
#include <QDebug>

DBusHRM::DBusHRM(QObject *parent) : QObject(parent)
{
    qDebug() << "Creating DBUS HRM";

    if (!QDBusConnection::sessionBus().registerService("org.sailfishos.heartrate")) {
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    } else {
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    }
}

void DBusHRM::setHRMService(HRMService *hrm)
{
    qDebug() << "Assigning HRM";

    m_hrm = hrm;
}

void DBusHRM::setMiBandService(MiBandService *mi)
{
    qDebug() << "Assigning MI";

    m_mi = mi;
}

void DBusHRM::start()
{
    qDebug() << "Starting DBUS HRM" << m_hrm;

    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(true);
    }
}

void DBusHRM::stop()
{
    qDebug() << "Stopping DBUS HRM" << m_hrm;

    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(false);
    }
}

int DBusHRM::heartRate()
{
    qDebug() << "Getting heartrate" << m_hrm->heartRate();

    if (m_hrm) {
        m_hrm->keepRealtimeHRMMeasurementAlive();
        return m_hrm->heartRate();
    }
    return 0;
}

int DBusHRM::batteryLevel()
{
    if (m_mi) {
        return m_mi->batteryInfo();
    }
    return 0;
}
