#ifndef GTSFIRMWAREINFO_H
#define GTSFIRMWAREINFO_H

#include "huamifirmwareinfo.h"

class GtsFirmwareInfo : public HuamiFirmwareInfo
{
public:
    explicit GtsFirmwareInfo(const QByteArray &bytes);

    bool supportedOnDevice(const QString &device) const override;

private:
    void determineFirmwareType();
    void determineFirmwareVersion();

    QMap<uint16_t, QString> m_crcMap;

    void populateCrcMap();
};

#endif // GTSFIRMWAREINFO_H
