#include "gts2device.h"

Gts2Device::Gts2Device(const QString &pairedName, QObject *parent) : GtsDevice(pairedName, parent)
{
    m_ActivitySampleSize = 8;
}
