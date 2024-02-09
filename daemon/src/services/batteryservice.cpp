#include "batteryservice.h"

const char* BatteryService::UUID_SERVICE_BATTERY  = "0000180f-0000-1000-8000-00805f9b34fb";
const char* BatteryService::UUID_CHARACTERISTIC_BATTERY_LEVEL = "00002a19-0000-1000-8000-00805f9b34fb";

BatteryService::BatteryService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_BATTERY, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &BatteryService::characteristicRead);
}

void BatteryService::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;

    readAsync(UUID_CHARACTERISTIC_BATTERY_LEVEL);
}

void BatteryService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Read:" << characteristic << value;
    if (characteristic == UUID_CHARACTERISTIC_BATTERY_LEVEL) {
        m_batteryLevel = value[0];
        emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(m_batteryLevel));
    } else {
        qWarning() << "Unknown value";
    }
}

int BatteryService::batteryLevel() const
{
    qDebug() << Q_FUNC_INFO << m_batteryLevel;
    return m_batteryLevel;
}
