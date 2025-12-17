#ifndef ABSTRACTFIRMWAREINFO_H
#define ABSTRACTFIRMWAREINFO_H

#include <QObject>
#include <QString>
#include <QMap>
#include <qble/qbleservice.h> //for macros

class AbstractFirmwareInfo : public QObject
{
    Q_OBJECT

public:
    AbstractFirmwareInfo();
    enum Type {
    Invalid = -1,
        Firmware = 0,
        Font = 1,
        Font_Latin = 11,
        GPS = 3,
        GPS_CEP = 4,
        GPS_UIHH = -4,
        GPS_ALMANAC = 5,
        Res = 2,
        Res_Compressed = 130,
        Watchface = 8,
        Bootloader = 100
    };
    Q_ENUM(Type)

    QString version() const;
    Type type() const;
    uint16_t getCrc16() const;
    uint32_t getCrc32() const;
    virtual bool supportedOnDevice(const QString &device) const = 0;
    QByteArray bytes() const;

protected:
    QByteArray m_bytes;
    uint16_t m_crc16;
    uint32_t m_crc32;
    Type m_type;
    QString m_version;
    QString m_fileName;

    void calculateCRC16();
    void calculateCRC32();

};

#endif // ABSTRACTFIRMWAREINFO_H
