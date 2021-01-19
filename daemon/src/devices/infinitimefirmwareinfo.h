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
    const uint8_t FW_HEADER[4]{ // DFU Zip file
            0x3D, 0xB8, 0xF3, 0x96
    };
    const uint8_t BOOTLOADER_HEADER[8]{
            0x00, 0x00, 0x01, 0xD9, 0x20, 0x00, 0x00, 0x00
    };
};

#endif // INFINITIMEFWINFO_H
