#ifndef INFINITIMEFWINFO_H
#define INFINITIMEFWINFO_H

#include "abstractfirmwareinfo.h"

class InfinitimeFirmwareInfo : public AbstractFirmwareInfo
{
public:;
    explicit InfinitimeFirmwareInfo(const QByteArray &bytes);
    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    const uint8_t ZIP_HEADER[4]{ // Zip file
            0x50, 0x4b, 0x03, 0x04
    };
};

#endif // INFINITIMEFWINFO_H
