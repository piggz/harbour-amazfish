#include "gtr2device.h"

Gtr2Device::Gtr2Device(const QString &pairedName, QObject *parent) : GtrDevice(pairedName,parent)
{
    m_ActivitySampleSize = 8;
}
