#include "bipinfoservice.h"

BipInfoService::BipInfoService(QObject *parent) : BipService("{0000180a-0000-1000-8000-00805f9b34fb}", parent)
{
    connect(this, &BipService::characteristicRead, this, &BipInfoService::characteristicRead);
}

void BipInfoService::refreshInformation()
{
    if (ready()) {
        QLowEnergyCharacteristic c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_INFO_SERIAL_NO));
        service()->readCharacteristic(c);
        c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_INFO_HARDWARE_REV));
        service()->readCharacteristic(c);
        c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_INFO_SOFTWARE_REV));
        service()->readCharacteristic(c);
        c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_INFO_SYSTEM_ID));
        service()->readCharacteristic(c);
        c = service()->characteristic(QBluetoothUuid(UUID_CHARACTERISTIC_INFO_PNP_ID));
        service()->readCharacteristic(c);
    }

}

void BipInfoService::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;
    if (characteristic.uuid().toString() == UUID_CHARACTERISTIC_INFO_SERIAL_NO) {
        m_serialNumber = value;
        emit serialNumberChanged();
    } else if (characteristic.uuid().toString() == UUID_CHARACTERISTIC_INFO_HARDWARE_REV) {
        m_hardwareRevision = value;
        emit hardwareRevisionChanged();
    } else if (characteristic.uuid().toString() == UUID_CHARACTERISTIC_INFO_SOFTWARE_REV) {
        m_softwareRevision = value;
        emit softwareRevisionChanged();
    } else if (characteristic.uuid().toString() == UUID_CHARACTERISTIC_INFO_SYSTEM_ID) {
        m_systemId = value;
        emit systemIdChanged();
    } else if (characteristic.uuid().toString() == UUID_CHARACTERISTIC_INFO_PNP_ID) {
        m_pnpId = value;
        emit pnpIdChanged();
    } else {
        qDebug() << "Unknown value";
    }
}

QString BipInfoService::serialNumber() const
{
    return m_serialNumber;
}


QString BipInfoService::hardwareRevision() const
{
    return m_hardwareRevision;
}


QString BipInfoService::softwareRevision() const
{
    return m_softwareRevision;
}


QString BipInfoService::systemId() const
{
    return m_systemId;
}


QString BipInfoService::pnpId() const
{
    return m_pnpId;
}
