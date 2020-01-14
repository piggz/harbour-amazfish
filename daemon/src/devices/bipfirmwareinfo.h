#ifndef HUAMIFIRMWAREINFO_H
#define HUAMIFIRMWAREINFO_H

#include <QByteArray>
#include <QObject>
#include <QString>
#include <QMap>

class HuamiFirmwareInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
    
public:
    HuamiFirmwareInfo(const QByteArray &bytes);
    
    enum Type {
    Invalid = -1,
        Firmware = 0,
        Font = 1,
        Font_Latin = 11,
        GPS = 3,
        GPS_CEP = 4,
        GPS_ALMANAC = 5,
        Res = 2,
        Res_Compressed = 130,
        Watchface = 8
    };
    
    

    const char RES_HEADER[5]{ // HMRES resources file (*.res)
            0x48, 0x4d, 0x52, 0x45, 0x53
    };

    const char NEWRES_HEADER[5] = { // NERES resources file (*.res)
            0x4e, 0x45, 0x52, 0x45, 0x53
    };

    const char WATCHFACE_HEADER[6] = {
            0x48, 0x4d, 0x44, 0x49, 0x41, 0x4c
    };

    const char FT_HEADER[4] = { // HMZK font file (*.ft, *.ft.xx)
            0x48, 0x4d, 0x5a, 0x4b
    };

    const char NEWFT_HEADER[4] = { // NEZK font file (*.ft, *.ft.xx)
            0x4e, 0x45, 0x5a, 0x4b
    };

    const char GPS_HEADER[16] = {
            0xcb, 0x51, 0xc1, 0x30, 0x41, 0x9e, 0x5e, 0xd3,
            0x51, 0x35, 0xdf, 0x66, 0xed, 0xd9, 0x5f, 0xa7
    };

    const char GPS_HEADER2[16] = {
            0x10, 0x50, 0x26, 0x76, 0x8f, 0x4a, 0xa1, 0x49,
            0xa7, 0x26, 0xd0, 0xe6, 0x4a, 0x21, 0x88, 0xd4
    };

    const char GPS_HEADER3[16] = {
            0xeb, 0xfa, 0xc5, 0x89, 0xf0, 0x5c, 0x2e, 0xcc,
            0xfa, 0xf3, 0x62, 0xeb, 0x92, 0xc6, 0xa1, 0xbb
    };

    const char GPS_HEADER4[16] = {
            0x0b, 0x61, 0x53, 0xed, 0x83, 0xac, 0x07, 0x21,
            0x8c, 0x36, 0x2e, 0x8c, 0x9c, 0x08, 0x54, 0xa6
    };

    // this is the same as Cor
   const char FW_HEADER[16] = {
            0x00, 0x98, 0x00, 0x20, 0xA5, 0x04, 0x00, 0x20, 0xAD, 0x04, 0x00, 0x20, 0xC5, 0x04, 0x00, 0x20
    };

   const char GPS_ALMANAC_HEADER[6] = { // probably wrong
            0xa0, 0x80, 0x08, 0x00, 0x8b, 0x07
    };

    const char GPS_CEP_HEADER[4]{ // probably wrong
            0x2a, 0x12, 0xa0, 0x02
    };
    
    int size();
    QByteArray bytes();
    QString version();
    Type type() const;
    uint16_t crc16() const;
    
private:
    
    QByteArray m_bytes;
    uint16_t m_crc16;
    Type m_type;
    QString m_version;
    QMap<uint16_t, QString> m_crcMap;

    void populateCrcMap();
    uint16_t calculateCRC16();
    Type determineFirmwareType();
    QString searchFirmwareVersion();
};

#endif // HUAMIFIRMWAREINFO_H
