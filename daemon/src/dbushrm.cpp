#include "dbushrm.h"
#include <QDebug>

DBusHRM::DBusHRM(HRMService * hrm, MiBandService *mi, QObject *parent) : QObject(parent)
{
    m_hrm = hrm;
    m_mi = mi;

    if (!QDBusConnection::sessionBus().registerService("org.sailfishos.heartrate")) {
        qDebug() << QDBusConnection::sessionBus().lastError().message();
    } else {
        QDBusConnection::sessionBus().registerObject("/", this, QDBusConnection::ExportAllSlots);
    }
}

void DBusHRM::start()
{
    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(true);
    }
}

void DBusHRM::stop()
{
    if (m_hrm) {
        return m_hrm->enableRealtimeHRMeasurement(false);
    }
}

int DBusHRM::heartRate()
{
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
