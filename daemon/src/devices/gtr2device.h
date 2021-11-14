#ifndef GTR2DEVICE_H
#define GTR2DEVICE_H

#include "gtrdevice.h"
#include <QObject>

class Gtr2Device : public GtrDevice
{
public:
    explicit Gtr2Device(const QString &pairedName, QObject *parent = nullptr);
};

#endif // GTR2DEVICE_H
