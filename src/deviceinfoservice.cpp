#include "deviceinfoservice.h"

const char* DeviceInfoService::UUID_SERVICE_DEVICEINFO  = "0000180a-0000-1000-8000-00805f9b34fb";

const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SERIAL_NO = "00002a25-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_HARDWARE_REV = "00002a27-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SOFTWARE_REV = "00002a28-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SYSTEM_ID = "00002a23-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_PNP_ID = "00002a50-0000-1000-8000-00805f9b34fb";


DeviceInfoService::DeviceInfoService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_DEVICEINFO, path, parent)
{
    qDebug() << "BipInfoService::BipInfoService";

    connect(this, &QBLEService::characteristicRead, this, &DeviceInfoService::characteristicRead);
}

void DeviceInfoService::refreshInformation()
{
    qDebug() << "BipInfoService::refreshInformation";

    readAsync(UUID_CHARACTERISTIC_INFO_SERIAL_NO);
    readAsync(UUID_CHARACTERISTIC_INFO_HARDWARE_REV);
    readAsync(UUID_CHARACTERISTIC_INFO_SOFTWARE_REV);
    readAsync(UUID_CHARACTERISTIC_INFO_SYSTEM_ID);
    readAsync(UUID_CHARACTERISTIC_INFO_PNP_ID);
}

void DeviceInfoService::characteristicRead(const QString &characteristic, const QByteArray &value)
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

QString DeviceInfoService::serialNumber() const
{
    return m_serialNumber;
}


QString DeviceInfoService::hardwareRevision() const
{
    return m_hardwareRevision;
}


QString DeviceInfoService::softwareRevision() const
{
    return m_softwareRevision;
}


QString DeviceInfoService::systemId() const
{
    return m_systemId;
}


QString DeviceInfoService::pnpId() const
{
    return m_pnpId;
}
