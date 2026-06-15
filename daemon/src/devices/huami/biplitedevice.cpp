#include "biplitedevice.h"
#include "biplitefirmwareinfo.h"

BipLiteDevice::BipLiteDevice(const QString &pairedName, QObject *parent) : BipDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
}

QString BipLiteDevice::deviceType() const
{
    return "amazfitbiplite";
}

void BipLiteDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;

    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &BipLiteDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &BipLiteDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &BipLiteDevice::serviceEvent, Qt::UniqueConnection);
        connect(mi, &QBLEService::characteristicChanged, this, &BipLiteDevice::characteristicChanged, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &BipDevice::authenticated, Qt::UniqueConnection);
        //connect(mi2, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(false);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(fw, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }
}

AbstractFirmwareInfo *BipLiteDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return new BipLiteFirmwareInfo(bytes);
}
