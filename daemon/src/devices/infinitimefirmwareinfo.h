#ifndef INFINITIMEFWINFO_H
#define INFINITIMEFWINFO_H

#include "abstractfirmwareinfo.h"

class InfinitimeFirmwareInfo : public AbstractFirmwareInfo
{
public:;
    InfinitimeFirmwareInfo(const QByteArray &bytes);
    virtual bool supportedOnDevice(const QString &device) const override;

protected:
    virtual void determineFirmwareType() override;
    virtual void determineFirmwareVersion() override;

private:
    const uint8_t FW_HEADER[8]{ // ELF FW image
            0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00
    };
};

#endif // INFINITIMEFWINFO_H
