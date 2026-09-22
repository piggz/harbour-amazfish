#include "neodevice.h"
#include "neofirmwareinfo.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "deviceinfoservice.h"
#include "bipfirmwareservice.h"
#include "hrmservice.h"
#include "alertnotificationservice.h"

#include <QDateTime>

NeoDevice::NeoDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << "Creating Neo Device" << pairedName;
}

QString NeoDevice::deviceType() const
{
    return "amazfitneo";
}

Amazfish::Features NeoDevice::supportedFeatures() const
{
    return Amazfish::Feature::FEATURE_HRM |
            Amazfish::Feature::FEATURE_WEATHER |
            Amazfish::Feature::FEATURE_ACTIVITY |
            Amazfish::Feature::FEATURE_STEPS |
            Amazfish::Feature::FEATURE_ALARMS |
            Amazfish::Feature::FEATURE_ALERT;
}

Amazfish::DataTypes NeoDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_ACTIVITY | Amazfish::DataType::TYPE_HEART_RATE;
}

void NeoDevice::serviceEvent(uint8_t event)
{
    switch(event) {
    case MiBandService::EVENT_FIND_PHONE:
        emit deviceEvent(AbstractDevice::EVENT_FIND_PHONE);
        break;
    case MiBandService::EVENT_CANCEL_FIND_PHONE:
        emit deviceEvent(AbstractDevice::EVENT_CANCEL_FIND_PHONE);
        break;
    default:
        break;
    }
}

void NeoDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &NeoDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &NeoDevice::serviceEvent, Qt::UniqueConnection);
        connect(mi, &QBLEService::characteristicChanged, this, &NeoDevice::characteristicChanged, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(false);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(fw, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    QString revision = softwareRevision();
    qDebug() << Q_FUNC_INFO << "Neo Firmware: " << revision;
    m_ActivitySampleSize = 8;

}

AbstractFirmwareInfo *NeoDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return new NeoFirmwareInfo(bytes);
}

void NeoDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    return;
}

void NeoDevice::applyDeviceSetting(Amazfish::Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    if (s == Amazfish::Settings::SETTING_DEVICE_DISPLAY_ITEMS) {
        mi->setDisplayItemsNew();
    } else {
        HuamiDevice::applyDeviceSetting(s);
    }
}

void NeoDevice::sendWeather(CurrentWeather *weather)
{
    sendWeatherHuami(weather, true);
}

QStringList NeoDevice::supportedDisplayItems() const
{
    QStringList items;

    items << "steps";
    items << "weather";
    // items << "dnd"; Enabling this item causes conflict
    items << "stopwatch";
    items << "alarm";
    items << "pai";
    items << "hr";

    return items;
}
