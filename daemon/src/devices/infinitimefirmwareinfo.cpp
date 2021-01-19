#include "infinitimefirmwareinfo.h"

InfinitimeFirmwareInfo::InfinitimeFirmwareInfo(const QByteArray &bytes)
{
    m_bytes = bytes;

    calculateCRC16();
    determineFirmwareType();
    determineFirmwareVersion();

    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16;
}

bool InfinitimeFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return (device == "InfiniTime" || device == "Pinetime-JF") && m_type != Invalid && !m_version.contains("unknown");;
}

void InfinitimeFirmwareInfo::determineFirmwareType()
{
    qDebug() << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(FW_HEADER))) {
        m_type = Firmware;
    } else if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(BOOTLOADER_HEADER))) {
        m_type = Bootloader;
    }
}

void InfinitimeFirmwareInfo::determineFirmwareVersion()
{
    switch (m_type) {
    case Firmware:
        m_version = "FW ()";
        break;
    case Bootloader:
        m_version = "BL ()";
        break;
    default:
        m_version = "unknown";
    }
}
