#ifndef DBUSHRM_H
#define DBUSHRM_H

#include <QObject>
#include "services/hrmservice.h"

class DBusHRM : public QObject
{
    Q_OBJECT
public:
    explicit DBusHRM(HRMService *hrm, QObject *parent = nullptr);

    Q_SCRIPTABLE void start();
    Q_SCRIPTABLE void stop();
    Q_SCRIPTABLE int heartRate();

private:
    HRMService *m_hrm = nullptr;
};

#endif // DBUSHRM_H
