#include "abstractfirmwareinfo.h"

AbstractFirmwareInfo::AbstractFirmwareInfo()
{
    m_type = Invalid;
}

QString AbstractFirmwareInfo::version()
{
    return m_version;
}

AbstractFirmwareInfo::Type AbstractFirmwareInfo::type() const
{
    return m_type;
}

void AbstractFirmwareInfo::calculateCRC16() {
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

    m_crc16 = crc;
}

uint16_t AbstractFirmwareInfo::crc16() const
{
    return m_crc16;
}
