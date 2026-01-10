#ifndef GTRFIRMWAREINFO_H
#define GTRFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class GtrFirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit GtrFirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    QMap<uint16_t, QString> m_crcMap;

};

#endif // GTSFIRMWAREINFO_H
