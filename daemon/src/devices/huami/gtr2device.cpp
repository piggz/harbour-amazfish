#include "gtr2device.h"
#include "gtr2firmwareinfo.h"

Gtr2Device::Gtr2Device(const QString &pairedName, QObject *parent) : Gts2Device(pairedName,parent)
{
}

QString Gtr2Device::deviceType() const
{
    return "amazfitgtr2";
}

AbstractFirmwareInfo *Gtr2Device::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return new Gtr2FirmwareInfo(bytes);
}
