#ifndef ZEPPOSFIRMWAREINFO_H
#define ZEPPOSFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class ZeppOSFirmwareInfo : public HuamiFirmwareInfo
{
public:
    ZeppOSFirmwareInfo(const QByteArray &bytes, const QString &path);
    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();
    bool isEpoZip();
    bool isBrmZip();
    bool buildEpoUIHH();
    bool buildBrmUIHH();
};

#endif // ZEPPOSFIRMWAREINFO_H
