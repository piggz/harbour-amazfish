#include "dbushrm.h"

DBusHRM::DBusHRM(HRMService * hrm, QObject *parent) : QObject(parent)
{
    m_hrm = hrm;
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
        return m_hrm->heartRate();
    }
    return 0;
}
