#include "deviceinfoservice.h"

const char* DeviceInfoService::UUID_SERVICE_DEVICEINFO  = "0000180a-0000-1000-8000-00805f9b34fb";

const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_MODEL_NO = "00002a24-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SERIAL_NO = "00002a25-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_FW_REVISION = "00002a26-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_HARDWARE_REV = "00002a27-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SOFTWARE_REV = "00002a28-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_MANUFACTURER_NAME = "00002a29-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_SYSTEM_ID = "00002a23-0000-1000-8000-00805f9b34fb";
const char* DeviceInfoService::UUID_CHARACTERISTIC_INFO_PNP_ID = "00002a50-0000-1000-8000-00805f9b34fb";

DeviceInfoService::DeviceInfoService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_DEVICEINFO, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicRead, this, &DeviceInfoService::characteristicRead);
}

void DeviceInfoService::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;

    readAsync(UUID_CHARACTERISTIC_INFO_SERIAL_NO);
    readAsync(UUID_CHARACTERISTIC_INFO_HARDWARE_REV);
    readAsync(UUID_CHARACTERISTIC_INFO_SOFTWARE_REV);
    readAsync(UUID_CHARACTERISTIC_INFO_SYSTEM_ID);
    readAsync(UUID_CHARACTERISTIC_INFO_PNP_ID);
    readAsync(UUID_CHARACTERISTIC_INFO_MODEL_NO);
    readAsync(UUID_CHARACTERISTIC_INFO_FW_REVISION);
    readAsync(UUID_CHARACTERISTIC_INFO_MANUFACTURER_NAME);
}

void DeviceInfoService::characteristicRead(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Read:" << characteristic << value;

    if (characteristic == UUID_CHARACTERISTIC_INFO_SERIAL_NO) {
        m_serialNumber = value;
        emit informationChanged(AbstractDevice::INFO_SERIAL, m_serialNumber);
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_HARDWARE_REV) {
        m_hardwareRevision = value;
        emit informationChanged(AbstractDevice::INFO_HWVER, m_hardwareRevision);
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_SOFTWARE_REV) {
        m_softwareRevision = value;
        emit informationChanged(AbstractDevice::INFO_SWVER, m_softwareRevision);
    } else if (characteristic  == UUID_CHARACTERISTIC_INFO_SYSTEM_ID) {
        m_systemId = value;
        emit informationChanged(AbstractDevice::INFO_SYSTEMID, m_systemId);
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_PNP_ID) {
        m_pnpId = value;
        emit informationChanged(AbstractDevice::INFO_PNPID, m_pnpId);
    }  else if (characteristic == UUID_CHARACTERISTIC_INFO_MODEL_NO) {
        m_model = value;
        emit informationChanged(AbstractDevice::INFO_MODEL, m_model);
    } else if (characteristic  == UUID_CHARACTERISTIC_INFO_FW_REVISION) {
        m_fwRevision = value;
        emit informationChanged(AbstractDevice::INFO_FW_REVISION, m_fwRevision);
    } else if (characteristic == UUID_CHARACTERISTIC_INFO_MANUFACTURER_NAME) {
        m_manufacturer = value;
        emit informationChanged(AbstractDevice::INFO_MANUFACTURER, m_manufacturer);
    } else {
        qDebug() << Q_FUNC_INFO << "Unknown value";
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

QString DeviceInfoService::readSoftwareRevisionSync()
{
    return readValue(UUID_CHARACTERISTIC_INFO_SOFTWARE_REV);
}

QString DeviceInfoService::modelNumber() const
{
    return m_model;
}

QString DeviceInfoService::fwRevision() const
{
    return m_fwRevision;
}

QString DeviceInfoService::manufacturerName() const
{
    return m_manufacturer;
}
