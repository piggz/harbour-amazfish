#include "infinitimefirmwareinfo.h"

InfinitimeFirmwareInfo::InfinitimeFirmwareInfo(const QByteArray &bytes)
{
    m_bytes = bytes;

    calculateCRC16();
    determineFirmwareType();
    determineFirmwareVersion();

    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16 << m_crc16x;
}

bool InfinitimeFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return device == "InfiniTime" && m_type != Invalid && !m_version.contains("unknown");;
}

void InfinitimeFirmwareInfo::determineFirmwareType()
{
    qDebug() << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(FW_HEADER))) {
        m_type = Firmware;
    }
}

void InfinitimeFirmwareInfo::determineFirmwareVersion()
{
    switch (m_type) {
    case Firmware:
        m_version = "FW ()";
        break;
    default:
        m_version = "unknown";
    }
}
