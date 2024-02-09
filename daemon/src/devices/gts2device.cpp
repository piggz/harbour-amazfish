#include "gts2device.h"
#include "gts2firmwareinfo.h"

Gts2Device::Gts2Device(const QString &pairedName, QObject *parent) : GtsDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_ActivitySampleSize = 8;
}

QString Gts2Device::deviceType() const
{
    return "amazfitgts2";
}

QStringList Gts2Device::supportedDisplayItems() const
{
    QStringList items;

    items << "goal";
    items << "pai";
    items << "hr";
    items << "workout";
    items << "activity";
    items << "weather";
    items << "alarm";
    items << "events"; //schedule?
    items << "stress";
    items << "spo2";
    items << "music";
    items << "widgets";
    items << "settings";

    return items;
}

AbstractFirmwareInfo *Gts2Device::firmwareInfo(const QByteArray &bytes)
{
    return new Gts2FirmwareInfo(bytes);
}

void Gts2Device::incomingCall(const QString &caller)
{
    //GTS2 handles call notificaion as a BT headset type device
    //using a seperate BT connection
    return;
}

void Gts2Device::initialise()
{
    qDebug() << Q_FUNC_INFO;
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &Gts2Device::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &Gts2Device::serviceEvent, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
    }

    if (mi2) {
        mi2->initialise(false);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(fw, &BipFirmwareService::downloadProgress, this, &HuamiDevice::downloadProgress, Qt::UniqueConnection);
        connect(fw, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }
}
