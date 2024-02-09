#ifndef BIPLITEFIRMWAREINFO_H
#define BIPLITEFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class BipLiteFirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit BipLiteFirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    QMap<uint16_t, QString> m_crcMap;

    void populateCrcMap();

};

#endif // BIPLITEFIRMWAREINFO_H
