#ifndef BipFirmwareInfo_H
#define BipFirmwareInfo_H

#include <QByteArray>
#include <QString>
#include <QMap>
#include "huamifirmwareinfo.h"

class BipFirmwareInfo final : public HuamiFirmwareInfo
{
public:
    explicit BipFirmwareInfo(const QByteArray &bytes);
    
    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    QMap<uint16_t, QString> m_crcMap;

    void populateCrcMap();
};

#endif // BipFirmwareInfo_H
