#include "gtrdevice.h"
#include "gtrfirmwareinfo.h"
#include <QTimer>
#include "mibandservice.h"
#include "miband2service.h"
#include "deviceinfoservice.h"
#include "bipfirmwareservice.h"
#include "hrmservice.h"

GtrDevice::GtrDevice(const QString &pairedName, QObject *parent) : GtsDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEDevice::propertiesChanged, this, &GtrDevice::onPropertiesChanged, Qt::UniqueConnection);
}

QString GtrDevice::deviceType() const
{
    return "amazfitgtr";
}

bool GtrDevice::is47mm(const QString &version) const
{
    return version >= "1.0.0.00" && version < "1.6.0.00";
}

AbstractFirmwareInfo *GtrDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    return new GtrFirmwareInfo(bytes);
}

void GtrDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &GtrDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &GtrDevice::serviceEvent, Qt::UniqueConnection);
        connect(mi, &QBLEService::characteristicChanged, this, &GtrDevice::characteristicChanged, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);
        //connect(mi2, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
    }

    if (mi2) {
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
    if (revision > "1.3.5.79" || // For GTR 47mm
            (!is47mm(revision) && revision >= "0.1.1.15")) { // for GTR 32mm with a different version scheme
        qDebug() << Q_FUNC_INFO << "GTR with new FW";
        m_ActivitySampleSize = 8;
    }
}
