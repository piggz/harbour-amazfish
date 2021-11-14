#ifndef GTS2DEVICE_H
#define GTS2DEVICE_H

#include "gtsdevice.h"

class Gts2Device : public GtsDevice
{
    Q_OBJECT
public:
    explicit Gts2Device(const QString &pairedName, QObject *parent = nullptr);
};

#endif // GTS2DEVICE_H
