#ifndef HRMSERVICE_H
#define HRMSERVICE_H

#include "qble/qbleservice.h"
#include "abstractdevice.h"

/*
{0000180d-0000-1000-8000-00805f9b34fb} Heart rate service
--00002a37-0000-1000-8000-00805f9b34fb //Heart rate measuremnet
--00002a39-0000-1000-8000-00805f9b34fb //Heart rate control point
*/
class HRMService : public QBLEService
{
    Q_OBJECT

public:
    HRMService(const QString &path, QObject *parent, bool handleDataExternally = false);
    static const char *UUID_SERVICE_HRM;
    static const char *UUID_CHARACTERISTIC_HRM_MEASUREMENT;
    static const char *UUID_CHARACTERISTIC_HRM_CONTROL;

    const uint8_t COMMAND_SET_HR_SLEEP = 0x0;
    const uint8_t COMMAND_SET_HR_CONTINUOUS_DISABLE[3] = {0x15, 0x1, 0x00};
    const uint8_t COMMAND_SET_HR_CONTINUOUS_ENABLE[3] = {0x15, 0x1, 0x01};
    const uint8_t COMMAND_SET_HR_MANUAL_DISABLE[3] = {0x15, 0x2, 0x00};
    const uint8_t COMMAND_SET_HR_MANUAL_ENABLE[3] = {0x15, 0x2, 0x01};
    const uint8_t COMMAND_SET_PERIODIC_HR_MEASUREMENT_INTERVAL = 0x14;
    const uint8_t COMMAND_ENABLE_HR_SLEEP_MEASUREMENT[3] = {0x15, 0x00, 0x01};
    const uint8_t COMMAND_DISABLE_HR_SLEEP_MEASUREMENT[3] = {0x15, 0x00, 0x00};

    void enableRealtimeHRMeasurement(bool enable);
    void enableManualHRMeasurement(bool enable);
    void keepRealtimeHRMMeasurementAlive();
    int heartRate() const;
    void setAllDayHRM();
    void setHeartrateSleepSupport();

    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString &val);

private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    int m_heartRate = 0;
};

#endif // HRMSERVICE_H
