#ifndef HRMSERVICE_H
#define HRMSERVICE_H

#include "qble/qbleservice.h"

/*
{0000180d-0000-1000-8000-00805f9b34fb} Heart rate service
--00002a37-0000-1000-8000-00805f9b34fb //Heart rate measuremnet
--00002a39-0000-1000-8000-00805f9b34fb //Heart rate control point
*/
class HRMService : public QBLEService
{
    Q_OBJECT

public:
    HRMService(QObject *parent);
    static const char *UUID_CHARACTERISTIC_HRM_MEASUREMENT;
    static const char *UUID_CHARACTERISTIC_HRM_CONTROL;

    const char COMMAND_SET_HR_SLEEP = 0x0;
    const char COMMAND_SET_HR_CONTINUOUS_DISABLE[3] = {0x15, 0x1, 0x00};
    const char COMMAND_SET_HR_CONTINUOUS_ENABLE[3] = {0x15, 0x1, 0x01};
    const char COMMAND_SET_HR_MANUAL_DISABLE[3] = {0x15, 0x2, 0x00};
    const char COMMAND_SET_HR_MANUAL_ENABLE[3] = {0x15, 0x2, 0x01};

    Q_PROPERTY(int heartRate READ heartRate NOTIFY heartRateChanged())

    Q_INVOKABLE void enableRealtimeHRMeasurement(bool enable);
    Q_INVOKABLE void enableManualHRMeasurement(bool enable);
    Q_INVOKABLE int heartRate() const;

    Q_SIGNAL void heartRateChanged();
private:
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    int m_heartRate = 0;
};

#endif // HRMSERVICE_H
