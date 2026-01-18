#ifndef BATTERYSERVICE_H
#define BATTERYSERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class BatteryService : public QBLEService
{
    Q_OBJECT
public:
    BatteryService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_BATTERY;
    static const char* UUID_CHARACTERISTIC_BATTERY_LEVEL;

    Q_INVOKABLE void refreshInformation();

    int batteryLevel() const;

    Q_SIGNAL void informationChanged(Amazfish::Info key, const QString &val);

private:
    int m_batteryLevel;
    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
};

#endif // BATTERYSERVICE_H
