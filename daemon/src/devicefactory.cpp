#include "devicefactory.h"
#include "bipdevice.h"
#include "gtsdevice.h"
#include "biplitedevice.h"
#include "pinetimejfdevice.h"

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName)
{
    qDebug() << "DeviceFactory::createDevice: requested device of type:" << deviceName;
    if (deviceName == "Amazfit Bip Watch") {
        return new BipDevice(deviceName);
    }

    if (deviceName == "Amazfit GTS") {
        return new GtsDevice(deviceName);
    }

    if (deviceName == "Amazfit Bip Lite") {
        return new BipLiteDevice(deviceName);
    }

    if (deviceName == "Mi Smart Band 4") {
        return new BipLiteDevice(deviceName);
    }

    if (deviceName == "Pinetime-JF") {
        return new PinetimeJFDevice(deviceName);
    }

    qDebug() << "DeviceFactory::createDevice: no suitable devices found, creating a Bip device as default";
    return new BipDevice(deviceName);

    //!TODO allow the user to choose the device type
    //return nullptr;
}
