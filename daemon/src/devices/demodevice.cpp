#include "demodevice.h"

#include <QDebug>

DemoDevice::DemoDevice(const QString& pairedName, QObject* parent)
    : AbstractDevice(pairedName, parent)
{
}

Amazfish::Features DemoDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_HRM | Amazfish::Feature::FEATURE_WEATHER | Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS | Amazfish::Feature::FEATURE_ALARMS | Amazfish::Feature::FEATURE_ALERT
        | Amazfish::Feature::FEATURE_EVENT_REMINDER | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        | Amazfish::Feature::FEATURE_BUTTON_ACTION | Amazfish::Feature::FEATURE_SCREENSHOT
        | Amazfish::Feature::FEATURE_FILE_INSTALL | Amazfish::Feature::FEATURE_SPO2;
}

Amazfish::DataTypes DemoDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_DEBUGLOG | Amazfish::DataType::TYPE_ACTIVITY | Amazfish::DataType::TYPE_GPS_TRACK
        | Amazfish::DataType::TYPE_STRESS | Amazfish::DataType::TYPE_SPO2 | Amazfish::DataType::TYPE_PAI
        | Amazfish::DataType::TYPE_HEART_RATE | Amazfish::DataType::TYPE_SLEEP_RESPIRATORY_RATE
        | Amazfish::DataType::TYPE_TEMPERATURE | Amazfish::DataType::TYPE_SLEEP
        | Amazfish::DataType::TYPE_HUAMI_STATISTICS | Amazfish::DataType::TYPE_HRV;
}

QString DemoDevice::deviceType() const
{
    return "demo";
}

AbstractFirmwareInfo* DemoDevice::firmwareInfo(const QByteArray& bytes, const QString& fileName)
{
    Q_UNUSED(bytes);
    Q_UNUSED(fileName);
    return nullptr;
}

void DemoDevice::sendAlert(const Amazfish::WatchNotification& notification)
{
    qDebug() << Q_FUNC_INFO << notification.summary;
}

void DemoDevice::incomingCall(const QString& caller)
{
    qDebug() << Q_FUNC_INFO << caller;
}

void DemoDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
}

void DemoDevice::connectToDevice()
{
    // No Bluetooth: the watch is there at once.
    setConnectionState("authenticated");
}

void DemoDevice::disconnectFromDevice()
{
    setConnectionState("disconnected");
}

void DemoDevice::fetchData(Amazfish::DataTypes dataTypes)
{
    Q_UNUSED(dataTypes);
    emit message(tr("Demo device: nothing to download"));
}

void DemoDevice::refreshInformation()
{
    // The UI updates its values from these signals.
    const Amazfish::Info infos[] = { Amazfish::Info::INFO_SWVER, Amazfish::Info::INFO_HWVER, Amazfish::Info::INFO_SERIAL,
        Amazfish::Info::INFO_GPSVER, Amazfish::Info::INFO_BATTERY, Amazfish::Info::INFO_STEPS,
        Amazfish::Info::INFO_HEARTRATE, Amazfish::Info::INFO_MODEL, Amazfish::Info::INFO_FW_REVISION,
        Amazfish::Info::INFO_MANUFACTURER, Amazfish::Info::INFO_SPO2 };
    for (Amazfish::Info i : infos) {
        emit informationChanged(i, information(i));
    }
}

QString DemoDevice::information(Amazfish::Info i) const
{
    switch (i) {
    case Amazfish::Info::INFO_SWVER:
    case Amazfish::Info::INFO_FW_REVISION:
        return "1.0.0";
    case Amazfish::Info::INFO_HWVER:
    case Amazfish::Info::INFO_GPSVER:
        return "1.0";
    case Amazfish::Info::INFO_SERIAL:
        return "DEMO-0001";
    case Amazfish::Info::INFO_MODEL:
        return "Demo watch";
    case Amazfish::Info::INFO_MANUFACTURER:
        return "Amazfish";
    case Amazfish::Info::INFO_BATTERY:
        return "80";
    case Amazfish::Info::INFO_STEPS:
        return "7342";
    case Amazfish::Info::INFO_HEARTRATE:
        return "72";
    case Amazfish::Info::INFO_SPO2:
        return "98";
    default:
        return QString();
    }
}

QStringList DemoDevice::supportedDisplayItems() const
{
    return { "status", "pai", "hr", "workout", "weather", "alarm", "music", "timer", "settings" };
}

QBLEService* DemoDevice::drv_createService(const QString& uuid, const QString& path)
{
    Q_UNUSED(uuid);
    Q_UNUSED(path);
    return nullptr;
}
