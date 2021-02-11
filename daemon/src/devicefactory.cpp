#include "devicefactory.h"
#include "huamidevice.h"
#include "gtsdevice.h"
#include "biplitedevice.h"
#include "pinetimejfdevice.h"
#include "banglejsdevice.h"
#include "bipsdevice.h"

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName)
{
    qDebug() << "DeviceFactory::createDevice: requested device of type:" << deviceName;
    if (deviceName == "Amazfit Bip Watch") {
        return new BipDevice(deviceName);
    }

    if (deviceName == "Amazfit GTS" || deviceName == "Amazfit GTR") {
        return new GtsDevice(deviceName);
    }

    if (deviceName == "Amazfit Bip Lite") {
        return new BipLiteDevice(deviceName);
    }

    if (deviceName == "Amazfit Bip S") {
        return new BipSDevice(deviceName);
    }

    if (deviceName == "Amazfit Stratos 3") {
        return new GtsDevice(deviceName);
    }

    if (deviceName == "Mi Smart Band 4") {
        return new BipLiteDevice(deviceName);
    }

    if (deviceName == "InfiniTime" || deviceName == "Pinetime-JF") {
        return new PinetimeJFDevice(deviceName);
    }

    if (deviceName.startsWith("Bangle.js")) {
        return new BangleJSDevice(deviceName);
    }

    qDebug() << "DeviceFactory::createDevice: no suitable devices found, creating a Bip device as default";
    return new BipDevice(deviceName);

    //!TODO allow the user to choose the device type
    //return nullptr;
}
