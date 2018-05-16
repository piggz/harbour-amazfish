#include "bipinfoservice.h"

const char* BipInfoService::UUID_SERVICE_DEVICEINFO  = "{00001811-0000-1000-8000-00805f9b34fb}";

const char* BipInfoService::UUID_CHARACTERISTIC_INFO_SERIAL_NO = "{00002a25-0000-1000-8000-00805f9b34fb}";
const char* BipInfoService::UUID_CHARACTERISTIC_INFO_HARDWARE_REV = "{00002a27-0000-1000-8000-00805f9b34fb}";
const char* BipInfoService::UUID_CHARACTERISTIC_INFO_SOFTWARE_REV = "{00002a28-0000-1000-8000-00805f9b34fb}";
const char* BipInfoService::UUID_CHARACTERISTIC_INFO_SYSTEM_ID = "{00002a23-0000-1000-8000-00805f9b34fb}";
const char* BipInfoService::UUID_CHARACTERISTIC_INFO_PNP_ID = "{00002a50-0000-1000-8000-00805f9b34fb}";


BipInfoService::BipInfoService(QObject *parent) : QBLEService(UUID_SERVICE_DEVICEINFO, parent)
{
    connect(this, &QBLEService::characteristicRead, this, &BipInfoService::characteristicRead);
}

void BipInfoService::refreshInformation()
{
    readCharacteristic(UUID_CHARACTERISTIC_INFO_SERIAL_NO);
    readCharacteristic(UUID_CHARACTERISTIC_INFO_HARDWARE_REV);
    readCharacteristic(UUID_CHARACTERISTIC_INFO_SOFTWARE_REV);
    readCharacteristic(UUID_CHARACTERISTIC_INFO_SYSTEM_ID);
    readCharacteristic(UUID_CHARACTERISTIC_INFO_PNP_ID);
}

void BipInfoService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "Read:" << characteristic << value;
    if (characteristic == UUID_CHARACTERISTIC_INFO_SERIAL_NO) {
        m_serialNumber = value;
        emit serialNumberChanged();
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_HARDWARE_REV) {
        m_hardwareRevision = value;
        emit hardwareRevisionChanged();
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_SOFTWARE_REV) {
        m_softwareRevision = value;
        emit softwareRevisionChanged();
    } else if (characteristic  == UUID_CHARACTERISTIC_INFO_SYSTEM_ID) {
        m_systemId = value;
        emit systemIdChanged();
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_PNP_ID) {
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
