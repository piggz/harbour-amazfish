#include "mibandservice.h"

const char* MiBandService::UUID_SERVICE_MIBAND = "{0000FEE0-0000-1000-8000-00805f9b34fb}";
const char* MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION = "{00000003-0000-3512-2118-0009af100700}";

MiBandService::MiBandService(QObject *parent) : BipService(UUID_SERVICE_MIBAND, parent)
{
    connect(this, &BipService::characteristicChanged, this, &MiBandService::characteristicChanged);
}

void MiBandService::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "MiBand Changed:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;
    if (value.length() < 4) {
        return;
    }

    if (value[0] == CHAR_RESPONSE && value[1] == COMMAND_REQUEST_GPS_VERSION && value[2] == CHAR_SUCCESS) {
        m_gpsVersion = value.mid(3);
        qDebug() << "Got gps version = " << m_gpsVersion;
        emit gpsVersionChanged();
    }
}

void MiBandService::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;
}

void MiBandService::requestGPSVersion()
{
    qDebug() << "Ready" << ready();
    if (ready()) {
        QLowEnergyCharacteristic characteristic = service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION)));
        service()->writeCharacteristic(characteristic, QByteArray(&COMMAND_REQUEST_GPS_VERSION, 1), QLowEnergyService::WriteWithoutResponse);
    }
}


QString MiBandService::gpsVersion()
{
    return m_gpsVersion;
}
