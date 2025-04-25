#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include <QString>
#include "abstractdevice.h"

class DeviceFactory
{
public:
    static AbstractDevice* createDevice(const QString &deviceName, const QString &deviceType);
};

#endif
