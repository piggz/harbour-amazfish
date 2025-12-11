#include "zepposfirmwareinfo.h"


ZeppOSFirmwareInfo::ZeppOSFirmwareInfo(const QByteArray &bytes) : HuamiFirmwareInfo()
{
    m_bytes = bytes;

    calculateCRC16();
    calculateCRC32();
    determineFirmwareType();
    determineFirmwareVersion();

    qDebug() << Q_FUNC_INFO << m_type << m_version << m_crc16 << m_crc32;
}

bool ZeppOSFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return true;
}

void ZeppOSFirmwareInfo::determineFirmwareType()
{
    qDebug() << Q_FUNC_INFO << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(AGPS_UIHH_HEADER))) {
        m_type = GPS_UIHH;
    }
}

void ZeppOSFirmwareInfo::determineFirmwareVersion()
{
    QString version;

    switch (m_type) {
    case GPS_UIHH:
        version = "GPS_UIHH";
        break;
    case Watchface:
        version = "Watchface";
        break;
    default:
        version = "(unknown)";
    }

    m_version = version;
}
