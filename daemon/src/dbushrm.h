#ifndef DBUSHRM_H
#define DBUSHRM_H

#include <QObject>
#include "services/hrmservice.h"
#include "services/mibandservice.h"

class DBusHRM : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.sailfishos.heartrate")
public:
    explicit DBusHRM(HRMService *hrm, MiBandService *mi, QObject *parent = nullptr);

public Q_SLOTS:
    Q_SCRIPTABLE void start();
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE int heartRate();
    Q_SCRIPTABLE int batteryLevel();

private:
    HRMService *m_hrm = nullptr;
    MiBandService *m_mi = nullptr;
};

#endif // DBUSHRM_H
