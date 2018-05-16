#ifndef BIPDEVICE_H
#define BIPDEVICE_H

#include "qble/qbledevice.h"
#include "qble/qbleservice.h"

#include "bipinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"

class BipDevice : public QBLEDevice
{
    Q_OBJECT
public:
    BipDevice();
    static const char* UUID_SERVICE_ALERT_NOTIFICATION;
    static const char* UUID_SERVICE_MIBAND2;
    static const char* UUID_SERVICE_MIBAND;
    static const char* UUID_SERVICE_HRM;
    static const char* UUID_SERVICE_DEVICEINFO;

private:
    BipInfoService *m_infoService = nullptr;
    MiBandService *m_mibandService = nullptr;
    MiBand2Service *m_miband2Service = nullptr;
    AlertNotificationService *m_alertNotificationService = nullptr;
    HRMService *m_hrmService = nullptr;

    QList<QBLEService *>m_genericServices;

};

#endif // BIPDEVICE_H
