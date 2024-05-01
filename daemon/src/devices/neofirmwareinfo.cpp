#include "neofirmwareinfo.h"
#include <QDebug>

NeoFirmwareInfo::NeoFirmwareInfo(const QByteArray &bytes)
{
    populateCrcMap();

    m_bytes = bytes;

    calculateCRC16();
    calculateCRC32();
    determineFirmwareType();
    determineFirmwareVersion();

    qDebug() << Q_FUNC_INFO << m_type << m_version << m_crc16 << m_crc32;
}

void NeoFirmwareInfo::determineFirmwareType() {
    qDebug() << Q_FUNC_INFO << "Determining firmware type";
    m_type = Invalid;

    m_version = m_crcMap[m_crc16];
    qDebug() << Q_FUNC_INFO << "Version:" << m_version << "CRC:" << m_crc16;
    m_type = Firmware;
}

bool NeoFirmwareInfo::supportedOnDevice(const QString &device) const
{
    qDebug() << "Checking if device is supported: " << device;
    return device == "Amazfit Neo" && m_type != Invalid && !m_version.contains("unknown");
}

void NeoFirmwareInfo::determineFirmwareVersion()
{
    QString version = m_crcMap[m_crc16];

    version = "FW " + version;
    m_version = version;
}

void NeoFirmwareInfo::populateCrcMap()
{
    // firmware
    m_crcMap.insert(48669, "1.1.2.58");

    // Latin Firmware
    // font
    // m_crcMap.insert(27996, "18344,eb2f43f,126");

}
