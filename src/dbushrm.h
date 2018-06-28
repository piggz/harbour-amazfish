#ifndef DBUSHRM_H
#define DBUSHRM_H

#include <QObject>
#include "services/hrmservice.h"

class DBusHRM : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.sailfishos.heartrate")
public:
    explicit DBusHRM(HRMService *hrm, QObject *parent = nullptr);

public Q_SLOTS:
    Q_SCRIPTABLE void start();
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE int heartRate();

private:
    HRMService *m_hrm = nullptr;
};

#endif // DBUSHRM_H
