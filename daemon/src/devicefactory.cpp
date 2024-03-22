#include "devicefactory.h"
#include "asteroidosdevice.h"
#include "huamidevice.h"
#include "gtsdevice.h"
#include "neodevice.h"
#include "biplitedevice.h"
#include "pinetimejfdevice.h"
#include "banglejsdevice.h"
#include "bipsdevice.h"
#include "gts2device.h"
#include "gtrdevice.h"
#include "gtr2device.h"

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName)
{
    qDebug() << Q_FUNC_INFO <<": requested device of type:" << deviceName;

    if (deviceName == "Amazfit Bip Watch") {
        return new BipDevice(deviceName);
    }

    if (deviceName == "Amazfit GTS") {
        return new GtsDevice(deviceName);
    }

    if (deviceName == "Amazfit Neo") {
        return new NeoDevice(deviceName);
    }

    if (deviceName == "Amazfit GTS 2") {
        return new Gts2Device(deviceName);
    }

    if (deviceName == "Amazfit GTR") {
        return new GtrDevice(deviceName);
    }

    if (deviceName == "Amazfit GTR 2") {
        return new Gtr2Device(deviceName);
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

    QList<QString> asteroidDevices = {
        "AsteroidOS",
        "bass", "sturgeon", "narwhal", "sparrow", "dory",
        "lenok", "catfish", "carp", "smelt", "anthias",
        "pike", "sawfish", "ray", "firefish", "beluga", "skipjack",
        "koi", "mooneye", "swift", "nemo", "hoki",
        "minnow", "tetra", "sprat", "kingyo", "medaka"
    };

    for (auto iterator = asteroidDevices.begin(); iterator != asteroidDevices.end(); ++iterator) {
        if (deviceName == *iterator) {
            return new AsteroidOSDevice(deviceName);
        }
    }

    qDebug() << Q_FUNC_INFO << ": no suitable devices found, creating a Bip device as default";
    return new BipDevice(deviceName);

    //!TODO allow the user to choose the device type
    //return nullptr;
}
