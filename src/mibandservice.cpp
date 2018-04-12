#include "mibandservice.h"

MiBandService::MiBandService(QObject *parent) : BipService("{0000fee0-0000-1000-8000-00805f9b34fb}", parent)
{
    connect(this, &BipService::characteristicChanged, this, &MiBandService::characteristicChanged);
    connect(this, &BipService::readyChanged, this, &MiBandService::serviceReady);
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
        QLowEnergyCharacteristic c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION));
        service()->writeCharacteristic(c, QByteArray(&COMMAND_REQUEST_GPS_VERSION, 1), QLowEnergyService::WriteWithoutResponse);
    }
}

QString MiBandService::gpsVersion()
{
    return m_gpsVersion;
}

void MiBandService::serviceReady(bool r)
{
    if (r) {
        QLowEnergyCharacteristic c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_MIBAND_CONFIGURATION));
        QLowEnergyDescriptor notificationDesc = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (notificationDesc.isValid()) {
            qDebug() << "Requesting notifications";
            service()->writeDescriptor(notificationDesc, QByteArray::fromHex("0100"));
        }
    }
}
