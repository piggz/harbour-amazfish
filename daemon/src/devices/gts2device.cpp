#include "gts2device.h"

Gts2Device::Gts2Device(const QString &pairedName, QObject *parent) : GtsDevice(pairedName, parent)
{
    qDebug() << "Creating GTS2 Device";

    m_ActivitySampleSize = 8;
}
