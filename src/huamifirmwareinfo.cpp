#include "huamifirmwareinfo.h"
#include <QDebug>

HuamiFirmwareInfo::HuamiFirmwareInfo(const QByteArray &bytes)
{
    mBytes = bytes;
    m_type = determineFirmwareType();
    m_version = searchFirmwareVersion();
    m_crc16 = calculateCRC16();
    
    //qDebug() << mBytes;
    qDebug() << m_type << m_version << m_crc16;
}

HuamiFirmwareInfo::Type HuamiFirmwareInfo::determineFirmwareType() {
    qDebug() << "Determining firmware type";
    if (mBytes.startsWith(QByteArray(RES_HEADER, sizeof(RES_HEADER))) || mBytes.startsWith(QByteArray(NEWRES_HEADER, sizeof(NEWRES_HEADER)))) {
        if (mBytes.length() > 700000) { // dont know how to distinguish from Cor .res
            return HuamiFirmwareInfo::Invalid;
        }
        return HuamiFirmwareInfo::Res;
    }
    if (mBytes.startsWith(GPS_HEADER) || mBytes.startsWith(GPS_HEADER2) || mBytes.startsWith(GPS_HEADER3) || mBytes.startsWith(GPS_HEADER4)) {
        return HuamiFirmwareInfo::GPS;
    }
    if (mBytes.startsWith(GPS_ALMANAC_HEADER)) {
        return HuamiFirmwareInfo::GPS_ALMANAC;
    }
    if (mBytes.startsWith(GPS_CEP_HEADER)) {
        return HuamiFirmwareInfo::GPS_CEP;
    }
    if (mBytes.startsWith(QByteArray(FW_HEADER, sizeof(FW_HEADER)))) {
        m_version = searchFirmwareVersion();
        if (!m_version.isEmpty()) {
            if ((m_version >= "0.0.8.00") && (m_version <= "1.0.5.00")) {
                return HuamiFirmwareInfo::Firmware;
            }
        }
        return HuamiFirmwareInfo::Invalid;
    }
    qDebug() << "Checking agianst" << QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER));
    if (mBytes.startsWith(QByteArray(WATCHFACE_HEADER, sizeof(WATCHFACE_HEADER)))) {
        return HuamiFirmwareInfo::Watchface;
    }
    if (mBytes.startsWith(NEWFT_HEADER)) {
        if (mBytes[10] == 0x01) {
            return HuamiFirmwareInfo::Font;
        } else if (mBytes[10] == 0x02) {
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

    if (mBytes.length() > 0) {
        int len = mBytes.length();
        for (int i =0; i < len; ++i) {
            char byte = mBytes[i];
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
