#ifndef ABSTRACTFIRMWAREINFO_H
#define ABSTRACTFIRMWAREINFO_H

#include <QObject>
#include <QString>
#include <QMap>

class AbstractFirmwareInfo : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
public:
    AbstractFirmwareInfo();
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

    QString version() const;
    Type type() const;
    uint16_t crc16() const;
    virtual bool supportedOnDevice(const QString &device) const = 0;

protected:
    QByteArray m_bytes;
    uint16_t m_crc16;
    Type m_type;
    QString m_version;
    QMap<uint16_t, QString> m_crcMap;

    virtual void populateCrcMap() = 0;
    virtual void determineFirmwareType() = 0;
    virtual void determineFirmwareVersion() = 0;

    void calculateCRC16();
};

#endif // ABSTRACTFIRMWAREINFO_H
