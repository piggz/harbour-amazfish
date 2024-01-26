#include "gtrfirmwareinfo.h"
#include <QDebug>

GtrFirmwareInfo::GtrFirmwareInfo(const QByteArray &bytes)
{
    m_bytes = bytes;

    calculateCRC16();
    calculateCRC32();
    determineFirmwareType();
    determineFirmwareVersion();

    qDebug() << Q_FUNC_INFO << m_type << m_version << m_crc16 << m_crc32;
}

void GtrFirmwareInfo::determineFirmwareType() {
    qDebug() << Q_FUNC_INFO << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.indexOf(UCHARARR_TO_BYTEARRAY(NEWRES_HEADER)) == COMPRESSED_RES_HEADER_OFFSET_NEW) {
        m_type = Res_Compressed;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER2)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER3)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER4))) {
        m_type = GPS;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_ALMANAC_HEADER))) {
        m_type = GPS_ALMANAC;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(AGPS_UIHH_HEADER))) {
        m_type = GPS_UIHH;
    }
    if (m_bytes.indexOf(UCHARARR_TO_BYTEARRAY(FW_HEADER2)) == FW_OFFSET) {
        m_version = m_crcMap[m_crc16];
        qDebug() << Q_FUNC_INFO << "Version:" << m_version << "CRC:" << m_crc16;
        m_type = Firmware;
    }

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(WATCHFACE_HEADER)) || m_bytes.indexOf(UCHARARR_TO_BYTEARRAY(WATCHFACE_HEADER)) == COMPRESSED_RES_HEADER_OFFSET || m_bytes.indexOf(UCHARARR_TO_BYTEARRAY(WATCHFACE_HEADER)) == COMPRESSED_RES_HEADER_OFFSET_NEW) {
        m_type = Watchface;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(NEWFT_HEADER))) {
        if (m_bytes.at(10) == 0x01) {
            m_type = Font;
        } else if (m_bytes.at(10) == 0x02) {
            m_type = Font_Latin;
        }
    }
}

bool GtrFirmwareInfo::supportedOnDevice(const QString &device) const
{
    qDebug() << "Checking if device is supported: " << device;
    return device == "Amazfit GTR" && m_type != Invalid && !m_version.contains("unknown");
}

void GtrFirmwareInfo::determineFirmwareVersion()
{
    QString version = m_crcMap[m_crc16];

    if (!version.isEmpty()) {
        switch (m_type) {
        case Firmware:
            version = "FW " + version;
            break;
        case Res:
            version = "RES " + version;
            break;
        case Res_Compressed:
            version = "RES_COMPRESSED " + version;
            break;
        case Font:
            version = "FONT " + version;
            break;
        case Font_Latin:
            version = "FONT LATIN " + version;
            break;
        case GPS:
            version = "GPS " + version;
            break;
        default:
            version = "Invalid";
        }
    } else {
        switch (m_type) {
        case Firmware:
            version = "FW (unknown)";
            break;
        case Res:
            version = "RES (unknown)";
            break;
        case Res_Compressed:
            version = "RES_COMPRESSED (unknown)";
            break;
        case Font:
            version = "FONT (unknown)";
            break;
        case Font_Latin:
            version = "FONT LATIN (unknown)";
            break;
        case GPS:
            version = "GPS (unknown)";
            break;
        case GPS_CEP:
            version = "CEP";
            break;
        case GPS_UIHH:
            version = "GPS_UIHH";
            break;
        case GPS_ALMANAC:
            version = "ALM";
            break;
        case Watchface:
            version = "Watchface";
            break;
        default:
            version = "(unknown)";
        }
    }

    m_version = version;
}
