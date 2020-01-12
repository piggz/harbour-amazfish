#include "devicefactory.h"
#include "bipdevice.h"
#include "gtsdevice.h"

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName)
{
    qDebug() << "DeviceFactory::createDevice: requested device of type:" << deviceName;
    if (deviceName == "Amazfit Bip Watch") {
        return new BipDevice();
    }

    if (deviceName == "Amazfit GTS") {
        return new GtsDevice();
    }

    qDebug() << "DeviceFactory::createDevice: no suitable devices found, creating a Bip device as default";
    return new BipDevice();

    //!TODO allow the user to choose the device type
    //return nullptr;
}
