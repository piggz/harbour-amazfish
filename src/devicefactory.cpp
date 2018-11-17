#include "devicefactory.h"
#include "bipdevice.h"

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName)
{
    if (deviceName == "Amazfit Bip Watch") {
        return new BipDevice();
    }
    
    return nullptr;
}