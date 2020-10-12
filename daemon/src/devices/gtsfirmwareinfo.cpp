#include "gtsfirmwareinfo.h"
#include <QDebug>

GtsFirmwareInfo::GtsFirmwareInfo(const QByteArray &bytes)
{
    populateCrcMap();

    m_bytes = bytes;

    calculateCRC16();
    calculateCRC32();
    determineFirmwareType();
    determineFirmwareVersion();

    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16 << m_crc32;
}

void GtsFirmwareInfo::determineFirmwareType() {
    qDebug() << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.indexOf(QByteArray(NEWRES_HEADER, sizeof(NEWRES_HEADER))) == COMPRESSED_RES_HEADER_OFFSET_NEW) {
        m_type = Res_Compressed;
    }
    if (m_bytes.startsWith(QByteArray(GPS_HEADER, sizeof(GPS_HEADER))) || m_bytes.startsWith(QByteArray(GPS_HEADER2, sizeof(GPS_HEADER2))) || m_bytes.startsWith(QByteArray(GPS_HEADER3, sizeof(GPS_HEADER3))) || m_bytes.startsWith(QByteArray(GPS_HEADER4, sizeof(GPS_HEADER4)))) {
        m_type = GPS;
    }
    if (m_bytes.startsWith(QByteArray(GPS_ALMANAC_HEADER, sizeof(GPS_ALMANAC_HEADER)))) {
        m_type = GPS_ALMANAC;
    }
    if (m_bytes.startsWith(QByteArray(GPS_CEP_HEADER, sizeof(GPS_CEP_HEADER)))) {
        m_type = GPS_CEP;
    }
    if (m_bytes.indexOf(QByteArray(FW_HEADER, sizeof(FW_HEADER))) == FW_OFFSET) {
        m_version = m_crcMap[m_crc16];
        qDebug() << "Version:" << m_version << "CRC:" << m_crc16;
        m_type = Firmware;
    }

    if (m_bytes.startsWith(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER))) || m_bytes.indexOf(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER))) == COMPRESSED_RES_HEADER_OFFSET || m_bytes.indexOf(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER))) == COMPRESSED_RES_HEADER_OFFSET_NEW) {
        m_type = Watchface;
    }
    if (m_bytes.startsWith(QByteArray(NEWFT_HEADER, sizeof(NEWFT_HEADER)))) {
        if (m_bytes.at(10) == 0x01) {
            m_type = Font;
        } else if (m_bytes.at(10) == 0x02) {
            m_type = Font_Latin;
        }
    }
}

bool GtsFirmwareInfo::supportedOnDevice(const QString &device) const
{
    qDebug() << "Checking if device suppoerted: " << device;
    return device == "Amazfit GTS" && m_type != Invalid && !m_version.contains("unknown");
}

void GtsFirmwareInfo::determineFirmwareVersion()
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
            version = "CAP";
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

void GtsFirmwareInfo::populateCrcMap()
{
    // firmware
    m_crcMap.insert(48669, "0.0.8.71");

    // Latin Firmware

    // resources
    m_crcMap.insert(11789, "0.0.8.71");

    // gps
    m_crcMap.insert(62532, "18344,eb2f43f,126");

    // font
    m_crcMap.insert(27996, "18344,eb2f43f,126");

}
