#include "devicefactory.h"
#include "asteroidosdevice.h"
#include "huami/gtsdevice.h"
#include "huami/neodevice.h"
#include "huami/biplitedevice.h"
#include "banglejsdevice.h"
#include "huami/bipsdevice.h"
#include "huami/gts2device.h"
#include "huami/gtrdevice.h"
#include "huami/gtr2device.h"
#include "huami/zepposdevice.h"
#include "pebbledevice.h"
#include "pinetimejfdevice.h"
#include "dk08device.h"

#include <functional>
#include <QMetaEnum>
#include <QTextStream>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>

using DeviceCreator = std::function<AbstractDevice*(const QString &)>;

static const QMap<QString, DeviceCreator> deviceMap = {
    { "Amazfit Bip Watch", [](const QString &name) { return new BipDevice(name); } },
    { "Amazfit GTS", [](const QString &name) { return new GtsDevice(name); } },
    { "Amazfit Neo", [](const QString &name) { return new NeoDevice(name); } },
    { "Amazfit GTS 2", [](const QString &name) { return new Gts2Device(name); } },
    { "Amazfit GTR", [](const QString &name) { return new GtrDevice(name); } },
    { "Amazfit GTR 2", [](const QString &name) { return new Gtr2Device(name); } },
    { "Amazfit Bip Lite", [](const QString &name) { return new BipLiteDevice(name); } },
    { "Amazfit Bip S", [](const QString &name) { return new BipSDevice(name); } },
    { "Amazfit Stratos 3", [](const QString &name) { return new GtsDevice(name); } },
    { "Mi Smart Band 4", [](const QString &name) { return new BipLiteDevice(name); } },
    { "Amazfit Balance", [](const QString &name) { return new ZeppOSDevice(name); } },
    { "Amazfit GTR 3 Pro", [](const QString &name) { return new ZeppOSDevice(name); } },
    { "InfiniTime", [](const QString &name) { return new PinetimeJFDevice(name); } },
    { "Pebble", [](const QString &name) { return new PebbleDevice(name); } },
    { "Bangle.js", [](const QString &name) { return new BangleJSDevice(name); } },
    { "Kospet DK08", [](const QString &name) { return new DK08Device(name); } },
    { "AsteroidOS", [](const QString &name) { return new AsteroidOSDevice(name); } },

    { "Amazfit Cor", [](const QString &name) { return new BipLiteDevice(name); } },
    { "Mi Band 3", [](const QString &name) { return new BipLiteDevice(name); } },
    { "Mi Band 2", [](const QString &name) { return new BipLiteDevice(name); } },
    };

AbstractDevice* DeviceFactory::createDevice(const QString &deviceName, const QString &deviceType)
{
    qDebug() << Q_FUNC_INFO <<": requested device of type:" << deviceName << deviceType;

    if (deviceMap.contains(deviceType)) {
        return deviceMap.value(deviceType)(deviceName);
    }

    qWarning() << "DeviceCreator not found";
    return nullptr;
}

void DeviceFactory::printAvailableFeatures()
{
    for (auto it = deviceMap.begin(); it != deviceMap.end(); ++it) {
        QString type = it.key();

        // Create CamelCase filename from device name
        QString fileName;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0) // Qt 5
        QStringList parts = type.split(QRegExp("[\\s\\.\\-\\/]+"), QString::SkipEmptyParts);
#else // Qt 6+
        QStringList parts = type.split(QRegularExpression("[\\s\\.\\-\\/]+"), Qt::SkipEmptyParts);
#endif
        for (const QString &part : parts) {
            fileName += part.at(0).toUpper() + part.mid(1);
        }
        fileName += ".md";

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qWarning() << "Could not open file for writing:" << fileName;
            continue;
        }

        QTextStream out(&file);
        AbstractDevice* dev = it.value()(it.key());
        if (dev) {
            out << "---\n";
            out << "title: \"" << type << "\"\n";
            out << "image:\n";
            out << "link:\n";
            out << "support:\n";
            out << "features:" << "\n";
            Amazfish::Features supportedFeatures = dev->supportedFeatures();
            QMetaEnum featureEnum = QMetaEnum::fromType<Amazfish::Feature>();
            for (int i = 0; i < featureEnum.keyCount(); ++i) {
                int val = featureEnum.value(i);
                if (val == 0) {
                    continue; // Skip FEATURE_NONE
                }
                out << "  - id: \"" << featureEnum.key(i) << "\"" << "\n";
                if (supportedFeatures & static_cast<Amazfish::Feature>(val)) {
                    out << "    value: \"Y\"" << "\n";
                } else {
                    out << "    value: \"N\"" << "\n";
                }
            }

            Amazfish::DataTypes supportedDataTypes = dev->supportedDataTypes();
            QMetaEnum dataTypeEnum = QMetaEnum::fromType<Amazfish::DataType>();
            for (int i = 0; i < dataTypeEnum.keyCount(); ++i) {
                int val = dataTypeEnum.value(i);
                if (val == 0) {
                    continue; // Skip TYPE_NONE
                }

                out << "  - id: \"" << dataTypeEnum.key(i) << "\"" << "\n";
                if (supportedDataTypes & static_cast<Amazfish::DataType>(val)) {
                    out << "    value: \"Y\"" << "\n";
                } else {
                    out << "    value: \"N\"" << "\n";
                }
            }

            delete dev;
        }
        out << "---\n";
        file.close();
        qDebug() << "Exported features for" << type << "to" << fileName;
    }
}
