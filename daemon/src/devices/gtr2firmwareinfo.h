#ifndef GTR2FIRMWAREINFO_H
#define GTR2FIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class Gtr2FirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit Gtr2FirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

};

#endif // GTR2FIRMWAREINFO_H
