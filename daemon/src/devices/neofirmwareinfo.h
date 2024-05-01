#ifndef NEOFIRMWAREINFO_H
#define NEOFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class NeoFirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit NeoFirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    QMap<uint16_t, QString> m_crcMap;

    void populateCrcMap();
};

#endif // NEOFIRMWAREINFO_H
