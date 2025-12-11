#ifndef ZEPPOSFIRMWAREINFO_H
#define ZEPPOSFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class ZeppOSFirmwareInfo : public HuamiFirmwareInfo
{
public:
    ZeppOSFirmwareInfo(const QByteArray &bytes);
    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();
};

#endif // ZEPPOSFIRMWAREINFO_H
