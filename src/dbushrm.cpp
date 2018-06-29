#include "dbushrm.h"

DBusHRM::DBusHRM(HRMService * hrm, QObject *parent) : QObject(parent)
{
    m_hrm = hrm;
    
    if (!QDBusConnection::sessionBus().registerService("org.sailfishos.heartrate")) {
        fprintf(stderr, "%s\n",
                qPrintable(QDBusConnection::sessionBus().lastError().message()));
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
