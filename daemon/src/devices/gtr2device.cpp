#include "gtr2device.h"

Gtr2Device::Gtr2Device(const QString &pairedName, QObject *parent) : Gts2Device(pairedName,parent)
{
}

QString Gtr2Device::deviceType()
{
    return "amazfitgtr2";
}
