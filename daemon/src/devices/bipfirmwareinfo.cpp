#include "huamifirmwareinfo.h"
#include <QDebug>

HuamiFirmwareInfo::HuamiFirmwareInfo(const QByteArray &bytes)
{
    populateCrcMap();

    m_bytes = bytes;

    m_crc16 = calculateCRC16();
    m_type = determineFirmwareType();
    m_version = searchFirmwareVersion();
    
    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16;
}

HuamiFirmwareInfo::Type HuamiFirmwareInfo::determineFirmwareType() {
    qDebug() << "Determining firmware type";
    if (m_bytes.startsWith(QByteArray(RES_HEADER, sizeof(RES_HEADER))) || m_bytes.startsWith(QByteArray(NEWRES_HEADER, sizeof(NEWRES_HEADER)))) {
        if (m_bytes.length() > 700000) { // dont know how to distinguish from Cor .res
            return HuamiFirmwareInfo::Invalid;
        }
        return HuamiFirmwareInfo::Res;
    }
    if (m_bytes.startsWith(QByteArray(GPS_HEADER, sizeof(GPS_HEADER))) || m_bytes.startsWith(QByteArray(GPS_HEADER2, sizeof(GPS_HEADER2))) || m_bytes.startsWith(QByteArray(GPS_HEADER3, sizeof(GPS_HEADER3))) || m_bytes.startsWith(QByteArray(GPS_HEADER4, sizeof(GPS_HEADER4)))) {
        return HuamiFirmwareInfo::GPS;
    }
    if (m_bytes.startsWith(QByteArray(GPS_ALMANAC_HEADER, sizeof(GPS_ALMANAC_HEADER)))) {
        return HuamiFirmwareInfo::GPS_ALMANAC;
    }
    if (m_bytes.startsWith(QByteArray(GPS_CEP_HEADER, sizeof(GPS_CEP_HEADER)))) {
        return HuamiFirmwareInfo::GPS_CEP;
    }
    if (m_bytes.startsWith(QByteArray(FW_HEADER, sizeof(FW_HEADER)))) {
        m_version = m_crcMap[m_crc16];
        qDebug() << "Version:" << m_version << "CRC:" << m_crc16;

        if (!m_version.isEmpty()) {
            if ((m_version >= "0.0.8.00") && (m_version <= "1.2.0.00")) {
                return HuamiFirmwareInfo::Firmware;
            }
        }
        return HuamiFirmwareInfo::Invalid;
    }

    if (m_bytes.startsWith(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER)))) {
        return HuamiFirmwareInfo::Watchface;
    }
    if (m_bytes.startsWith(QByteArray(NEWFT_HEADER, sizeof(NEWFT_HEADER)))) {
        if (m_bytes.at(10) == 0x01) {
            return HuamiFirmwareInfo::Font;
        } else if (m_bytes.at(10) == 0x02) {
            return HuamiFirmwareInfo::Font_Latin;
        }
    }
    return HuamiFirmwareInfo::Invalid;
}

QString HuamiFirmwareInfo::searchFirmwareVersion() {
    /*
    ByteBuffer buf = ByteBuffer.wrap(fwbytes);
    buf.order(ByteOrder.BIG_ENDIAN);
    while (buf.remaining() > 3) {
        int word = buf.getInt();
        if (word == 0x5625642e) {
            word = buf.getInt();
            if (word == 0x25642e25) {
                word = buf.getInt();
                if (word == 0x642e2564) {
                    word = buf.getInt();
                    if (word == 0x00000000) {
                        byte version[] = new byte[8];
                        buf.get(version);
                        return new String(version);
                    }
                }
            }
        }
    }
    */
    return QString();
}

HuamiFirmwareInfo::Type HuamiFirmwareInfo::type() const
{
    return m_type;
}

uint16_t HuamiFirmwareInfo::calculateCRC16() {
    uint16_t crc = 0xFFFF;

    if (m_bytes.length() > 0) {
        int len = m_bytes.length();
        for (int i =0; i < len; ++i) {
            char byte = m_bytes[i];
            crc = (crc >> 8) | (crc << 8);
            crc ^= byte;
            crc ^= ((unsigned char) crc) >> 4;
            crc ^= crc << 12;
            crc ^= (crc & 0xFF) << 5;
        }
    }

    return crc;
}

uint16_t HuamiFirmwareInfo::crc16() const
{
    return m_crc16;
}

QString HuamiFirmwareInfo::version()
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
        }
    } else {
        switch (m_type) {
        case Firmware:
            version = "FW (unknown)";
            break;
        case Res:
            version = "RES " + QString::number(m_bytes[5]);
            break;
        case Res_Compressed:
            version = "RES " + QString::number(m_bytes[14]);
            break;
        case Font:
            version = "FONT " + QString::number(m_bytes[4]);
            break;
        case Font_Latin:
            version = "FONT LATIN " + QString::number(m_bytes[4]);
            break;
        case GPS:
            version = "GPS (unknown)";
            break;
        case GPS_CEP:
            version = "CAP (unknown)";
            break;
        case GPS_ALMANAC:
            version = "ALM (unknown)";
            break;
        case Watchface:
            version = "Watchface (unknown)";
            break;
        default:
            version = "Invalid";
        }
    }

    return version;
}

void HuamiFirmwareInfo::populateCrcMap()
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

    // BipOS RES
    m_crcMap.insert(16303, "1.1.2.05 (BipOS 0.5)");
    m_crcMap.insert(61135, "1.1.2.05 (BipOS 0.5.1)");

}
