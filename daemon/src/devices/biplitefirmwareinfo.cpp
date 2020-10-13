#include "biplitefirmwareinfo.h"
#include <QDebug>

BipLiteFirmwareInfo::BipLiteFirmwareInfo(const QByteArray &bytes)
{
    populateCrcMap();

    m_bytes = bytes;

    calculateCRC16();
    determineFirmwareType();
    determineFirmwareVersion();

    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16;
}


void BipLiteFirmwareInfo::determineFirmwareType() {
    qDebug() << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(QByteArray(RES_HEADER, sizeof(RES_HEADER))) || m_bytes.startsWith(QByteArray(NEWRES_HEADER, sizeof(NEWRES_HEADER)))) {
        if (m_bytes.length() > 700000) { // dont know how to distinguish from Cor .res
            m_type = Invalid;
        }
        m_type = Res;
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
    if (m_bytes.startsWith(QByteArray(FW_HEADER, sizeof(FW_HEADER)))) {
        m_version = m_crcMap[m_crc16];
        qDebug() << "Version:" << m_version << "CRC:" << m_crc16;

        if (!m_version.isEmpty()) {
            if ((m_version >= "0.0.8.00") && (m_version <= "1.2.0.00")) {
                m_type = Firmware;
            }
        }
        m_type = Invalid;
    }

    if (m_bytes.startsWith(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER)))) {
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

bool BipLiteFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return device == "Amazfit Bip Lite" && m_type != Invalid && !m_version.contains("unknown");;
}

void BipLiteFirmwareInfo::determineFirmwareVersion()
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
            version = "RES " + version;
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
            version = "RES (unknown)" + QString::number(m_bytes[5]);
            break;
        case Res_Compressed:
            version = "RES (unknown)" + QString::number(m_bytes[14]);
            break;
        case Font:
            version = "FONT (unknown)" + QString::number(m_bytes[4]);
            break;
        case Font_Latin:
            version = "FONT LATIN (unknown)" + QString::number(m_bytes[4]);
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
            version = "Invalid";
        }
    }

    m_version = version;
}

void BipLiteFirmwareInfo::populateCrcMap()
{
    // firmware
    m_crcMap.insert(11059, "1.1.6.02");

    // Latin Firmware


    // resources
    m_crcMap.insert(57510, "1.1.6.02");

    // font
    m_crcMap.insert(61054, "8");
    m_crcMap.insert(59577, "9 (Latin)");

    // gps

}
