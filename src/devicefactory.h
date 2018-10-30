#ifndef DEVICEFACTORY_H
#define DEVICEFACTORY_H

#include <QString>
#include "abstractdevice.h"

class DeviceFactory
{
    DeviceFactory();
    
    static AbstractDevice* createDevice(const QString &deviceName);
};

#endif