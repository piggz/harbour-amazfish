#include "abstractfirmwareinfo.h"
#include <zlib.h>

AbstractFirmwareInfo::AbstractFirmwareInfo(): m_crc16(0), m_crc32(0)
{
    m_type = Invalid;
}

QString AbstractFirmwareInfo::version() const
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

void AbstractFirmwareInfo::calculateCRC16x()
{
    uint16_t crc = 0xFFFF;

    if (m_bytes.length() > 0) {
        int len = m_bytes.length();
        for (int i =0; i < len; ++i) {
            char byte = m_bytes[i];
            crc = (uint8_t) (crc >> 8) | (crc << 8);
            crc ^= byte;
            crc ^= (uint8_t) (crc & 0xFF) >> 4;
            crc ^= (crc << 8) << 4;
            crc ^= ((crc & 0xFF) << 4) << 1;
        }
    }

    m_crc16x = crc;
}

uint16_t AbstractFirmwareInfo::getCrc16() const
{
    return m_crc16;
}

uint16_t AbstractFirmwareInfo::getCrc16c() const
{
    return m_crc16x;
}

uint32_t AbstractFirmwareInfo::getCrc32() const
{
    return m_crc32;
}

QByteArray AbstractFirmwareInfo::bytes() const
{
    return m_bytes;
}

void AbstractFirmwareInfo::calculateCRC32()
{
    uint32_t crc = crc32(0L, Z_NULL, 0);

    char *bytes = m_bytes.data();
    int len = m_bytes.length();

    for (int i = 0; i < len; ++i)
    {
        crc = crc32(crc, (unsigned char*)bytes + i, 1);
    }
    m_crc32 = crc;
}
