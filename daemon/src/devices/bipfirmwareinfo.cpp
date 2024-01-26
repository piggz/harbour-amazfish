#include "bipfirmwareinfo.h"
#include <QDebug>

BipFirmwareInfo::BipFirmwareInfo(const QByteArray &bytes)
{
    populateCrcMap();

    m_bytes = bytes;

    calculateCRC16();
    determineFirmwareType();
    determineFirmwareVersion();
    
    qDebug() << Q_FUNC_INFO << m_type << m_version << m_crc16;
}

void BipFirmwareInfo::determineFirmwareType() {
    qDebug() << Q_FUNC_INFO << "Determining firmware type";
    m_type = Invalid;

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(RES_HEADER)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(NEWRES_HEADER))) {
        if (m_bytes.length() > 700000) { // dont know how to distinguish from Cor .res
            m_type = Invalid;
            return;
        }
        m_type = Res;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER2)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER3)) || m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_HEADER4))) {
        m_type = GPS;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_ALMANAC_HEADER))) {
        m_type = GPS_ALMANAC;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(GPS_CEP_HEADER))) {
        m_type = GPS_CEP;
    }
    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(FW_HEADER))) {
        m_version = m_crcMap[m_crc16];
        qDebug() << Q_FUNC_INFO << "Version:" << m_version << "CRC:" << m_crc16;
        m_type = Firmware;
    }

    if (m_bytes.startsWith(UCHARARR_TO_BYTEARRAY(WATCHFACE_HEADER))) {
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

bool BipFirmwareInfo::supportedOnDevice(const QString &device) const
{
    return device == "Amazfit Bip Watch" && m_type != Invalid && !m_version.contains("unknown");;
}

void BipFirmwareInfo::determineFirmwareVersion()
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

void BipFirmwareInfo::populateCrcMap()
{
    // firmware
    m_crcMap.insert(25257, "0.0.8.74");
    m_crcMap.insert(57724, "0.0.8.88");
    m_crcMap.insert(27668, "0.0.8.96");
    m_crcMap.insert(60173, "0.0.8.97");
    m_crcMap.insert(3462,  "0.0.8.98");
    m_crcMap.insert(55420, "0.0.9.14");
    m_crcMap.insert(39465, "0.0.9.26");
    m_crcMap.insert(27394, "0.0.9.40");
    m_crcMap.insert(24736, "0.0.9.49");
    m_crcMap.insert(49555, "0.0.9.59");
    m_crcMap.insert(28586, "0.1.0.08");
    m_crcMap.insert(26714, "0.1.0.11");
    m_crcMap.insert(64160, "0.1.0.17");
    m_crcMap.insert(21992, "0.1.0.26");
    m_crcMap.insert(43028, "0.1.0.27");
    m_crcMap.insert(59462, "0.1.0.33");
    m_crcMap.insert(55277, "0.1.0.39");
    m_crcMap.insert(47685, "0.1.0.43");
    m_crcMap.insert(2839,  "0.1.0.44");
    m_crcMap.insert(30229, "0.1.0.45");
    m_crcMap.insert(24302, "0.1.0.70");
    m_crcMap.insert(1333,  "0.1.0.80");
    m_crcMap.insert(12017, "0.1.0.86");
    m_crcMap.insert(8276,  "0.1.1.14");
    m_crcMap.insert(5914,  "0.1.1.17");
    m_crcMap.insert(6228,  "0.1.1.29");
    m_crcMap.insert(44223, "0.1.1.31");
    m_crcMap.insert(39726, "0.1.1.36");
    m_crcMap.insert(11062, "0.1.1.39");
    m_crcMap.insert(56670, "0.1.1.41");
    m_crcMap.insert(58736, "0.1.1.45");
    m_crcMap.insert(2602, "1.0.2.00");
    m_crcMap.insert(48207, "1.1.1.00");
    m_crcMap.insert(36157, "1.1.2.05");
    m_crcMap.insert(26444, "1.1.5.02");
    m_crcMap.insert(60002, "1.1.5.04");
    m_crcMap.insert(32576, "1.1.5.16");
    m_crcMap.insert(28893, "1.1.5.24");
    m_crcMap.insert(61710, "1.1.5.56");
    m_crcMap.insert(23387, "1.1.6.34");

    // Latin Firmware
    m_crcMap.insert(43876, "1.1.5.16 (latin)");
    m_crcMap.insert(52828, "1.1.5.36 (latin)");
    m_crcMap.insert(60625, "1.1.6.30 (Latin)");
    m_crcMap.insert(17913, "1.1.6.32 (Latin)");

    // resources
    m_crcMap.insert(12586, "0.0.8.74");
    m_crcMap.insert(34068, "0.0.8.88");
    m_crcMap.insert(59839, "0.0.8.96-98");
    m_crcMap.insert(50401, "0.0.9.14-26");
    m_crcMap.insert(22051, "0.0.9.40");
    m_crcMap.insert(46233, "0.0.9.49-0.1.0.11");
    m_crcMap.insert(12098, "0.1.0.17");
    m_crcMap.insert(28696, "0.1.0.26-27");
    m_crcMap.insert(5650,  "0.1.0.33");
    m_crcMap.insert(16117, "0.1.0.39-45");
    m_crcMap.insert(22506, "0.1.0.66-70");
    m_crcMap.insert(42264, "0.1.0.77-80");
    m_crcMap.insert(55934, "0.1.0.86-89");
    m_crcMap.insert(26587, "0.1.1.14-25");
    m_crcMap.insert(7446,  "0.1.1.29");
    m_crcMap.insert(47887, "0.1.1.31-36");
    m_crcMap.insert(14334, "0.1.1.39");
    m_crcMap.insert(21109, "0.1.1.41");
    m_crcMap.insert(23073, "0.1.1.45");
    m_crcMap.insert(59245, "1.0.2.00");
    m_crcMap.insert(20591, "1.1.2.05");
    m_crcMap.insert(5341,  "1.1.5.02-04");
    m_crcMap.insert(22662, "1.1.5.36");
    m_crcMap.insert(24045, "1.1.5.56");
    m_crcMap.insert(37677, "1.1.6.30-32");
    m_crcMap.insert(26735, "1.1.6.34");

    // gps
    m_crcMap.insert(61520, "9367,8f79a91,0,0,");
    m_crcMap.insert(8784,  "9565,dfbd8fa,0,0,");
    m_crcMap.insert(16716, "9565,dfbd8faf42,0");
    m_crcMap.insert(54154, "9567,8b05506,0,0,");
    m_crcMap.insert(15717, "15974,e61dd16,126");
    m_crcMap.insert(62532, "18344,eb2f43f,126");

    // font
    m_crcMap.insert(61054, "8");
    m_crcMap.insert(62291, "9 (old Latin)");
    m_crcMap.insert(59577, "9 (Latin)");

    // BipOS FW
    m_crcMap.insert(28373, "1.1.2.05 (BipOS 0.5)");
    m_crcMap.insert(62977, "1.1.2.05 (BipOS 0.5.1)");
    m_crcMap.insert(7677, "1.1.5.36 (BipOS 0.5.2)");

    // BipOS RES
    m_crcMap.insert(16303, "1.1.2.05 (BipOS 0.5)");
    m_crcMap.insert(61135, "1.1.2.05 (BipOS 0.5.1)");
    m_crcMap.insert(52051, "1.1.5.36 (BipOS 0.5.2)");

}
