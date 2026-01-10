#ifndef GTS2FIRMWAREINFO_H
#define GTS2FIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class Gts2FirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit Gts2FirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

};

#endif // GTS2FIRMWAREINFO_H
