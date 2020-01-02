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
    explicit DBusHRM(QObject *parent = nullptr);
    void setHRMService(HRMService *hrm);
    void setMiBandService(MiBandService *mi);

public Q_SLOTS:
    void start();
    void stop();
    int heartRate();
    int batteryLevel();

private:
    HRMService *m_hrm = nullptr;
    MiBandService *m_mi = nullptr;
};

#endif // DBUSHRM_H
