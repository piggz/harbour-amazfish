#ifndef DBUSHRM_H
#define DBUSHRM_H

#include <QObject>
#include "services/hrmservice.h"
#include "abstractdevice.h"

class DBusHRM : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.sailfishos.heartrate")
public:
    explicit DBusHRM(QObject *parent = nullptr);
    void setHRMService(HRMService *hrm);
    void setDevice(AbstractDevice *dev);

public Q_SLOTS:
    void start();
    void stop();
    int heartRate();
    int batteryLevel();

private:
    HRMService *m_hrm = nullptr;
    AbstractDevice *m_device = nullptr;
};

#endif // DBUSHRM_H
