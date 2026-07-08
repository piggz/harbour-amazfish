#include "mijialywsdservice.h"
#include <QDataStream>

const char* MijiaLywsdService::UUID_SERVICE_MIJIA_LYWSD  = "ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6";
const char* MijiaLywsdService::UUID_CHARACTERISTIC_LIVE_DATA = "ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6";

MijiaLywsdService::MijiaLywsdService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_MIJIA_LYWSD, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &MijiaLywsdService::characteristicRead);
    connect(this, &QBLEService::characteristicChanged, this, &MijiaLywsdService::characteristicChanged);
}

void MijiaLywsdService::enableNotifications()
{
    qDebug() << Q_FUNC_INFO;
    enableNotification(UUID_CHARACTERISTIC_LIVE_DATA);
}

void MijiaLywsdService::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    readAsync(UUID_CHARACTERISTIC_LIVE_DATA);
}

void MijiaLywsdService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    characteristicRead(characteristic, value);
}

void MijiaLywsdService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Read:" << characteristic << value.toHex();
    if (characteristic == UUID_CHARACTERISTIC_LIVE_DATA) {
        if (value.length() == 5) {
            QDataStream stream(value);
            stream.setByteOrder(QDataStream::LittleEndian);
            
            qint16 tempRaw;
            quint8 humidity;
            qint16 voltRaw;
            
            stream >> tempRaw >> humidity >> voltRaw;
            
            float temperature = tempRaw / 100.0f;
            float voltage = voltRaw / 1000.0f;
            
            qDebug() << "LYWSD03MMC Live Data - Temperature:" << temperature << "Humidity:" << humidity << "Voltage:" << voltage;
            
            emit informationChanged(Amazfish::Info::INFO_TEMPERATURE, QString::number(temperature, 'f', 2));
            emit informationChanged(Amazfish::Info::INFO_HUMIDITY, QString::number(humidity));
            emit informationChanged(Amazfish::Info::INFO_VOLTAGE, QString::number(voltage, 'f', 3));
            
        } else {
            qWarning() << "Invalid length for live data:" << value.length();
        }
    } else {
        qWarning() << "Unknown characteristic:" << characteristic;
    }
}
