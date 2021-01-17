#include "huamidevice.h"
#include "bipfirmwareinfo.h"
#include "updatefirmwareoperation.h"

#include <QtXml/QtXml>

HuamiDevice::HuamiDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    m_keyPressTimer = new QTimer(this);
    m_keyPressTimer->setInterval(500);
    connect(m_keyPressTimer, &QTimer::timeout, this, &HuamiDevice::buttonPressTimeout);
}

bool HuamiDevice::operationRunning()
{
    return QBLEDevice::operationRunning();
}

void HuamiDevice::sendWeather(CurrentWeather *weather)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        bool supportsConditionString = (softwareRevision() > "V0.0.8.74");
        mi->sendWeather(weather, supportsConditionString);
    }
}

int HuamiDevice::activitySampleSize()
{
    return m_ActivitySampleSize;
}

QString HuamiDevice::softwareRevision()
{
    if (m_softwareRevision.isEmpty()) {
        DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
        if (info) {
            m_softwareRevision = info->readSoftwareRevisionSync();
        } else {
            m_softwareRevision = "V0.0.0.00";
        }
    }
    return m_softwareRevision;
}

void HuamiDevice::refreshInformation()
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
         info->refreshInformation();
    }
    
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->requestGPSVersion();
        mi->requestBatteryInfo();
    }
}

QString HuamiDevice::information(Info i) const
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
     if (!info) {
        return QString();
    }
    
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return QString();
    }
    
    switch(i) {
    case INFO_SWVER:
        return info->softwareRevision();
        break;
    case INFO_HWVER:
        return info->hardwareRevision();
        break;
    case INFO_SERIAL:
        return info->serialNumber();
        break;
    case INFO_SYSTEMID:
        return info->systemId();
        break;
    case INFO_PNPID:
        return info->pnpId();
        break;
    case INFO_GPSVER:
        return mi->gpsVersion();
        break;
    case INFO_BATTERY:
        return QString::number(mi->batteryInfo());
        break;
    case INFO_STEPS:
        return QString::number(mi->steps());
        break;
    }    
    return QString();
}    

void HuamiDevice::stepsChanged()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(mi->steps()));
}

void HuamiDevice::batteryInfoChanged()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(mi->batteryInfo()));
}

void HuamiDevice::rebootWatch()
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (!fw) {
        return;
    }
    fw->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE, QByteArray(1, BipFirmwareService::COMMAND_FIRMWARE_REBOOT));

}

void HuamiDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->sendAlert(sender, subject, message);
    }
}

void HuamiDevice::incomingCall(const QString &caller)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->incomingCall(caller);
    }
}

void HuamiDevice::navigationRunning(bool running)
{
    QString msg;
    if (running) {
        msg = tr("Navigation Started");
    } else {
        msg = tr("Navigation Stopped");
    }
    sendAlert("navigation", msg, "");
}

void HuamiDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    sendAlert("navigation", tr("Progress") + ":" + QString::number(progress), narrative + "\n" + manDist);
}

void HuamiDevice::handleButtonPressed()
{
    m_buttonPresses++;
    m_keyPressTimer->stop();
    m_keyPressTimer->start();
}

void HuamiDevice::buttonPressTimeout()
{
    int presses = m_buttonPresses;
    m_buttonPresses = 0;
    m_keyPressTimer->stop();
    emit buttonPressed(presses);
}


void HuamiDevice::authenticated(bool ready)
{
    qDebug() << "BipInterface::authenticated:" << ready;

    if (ready) {
        m_needsAuth = false;

        MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
        if (mi){
            mi->setCurrentTime();
            mi->setLanguage();
            mi->setDateDisplay();
            mi->setTimeFormat();
            mi->setUserInfo();
            mi->setDisplayCaller();
            mi->setAlertFitnessGoal();
            mi->setDistanceUnit();
            mi->setWearLocation();
            mi->setFitnessGoal();
            mi->setDisplayItems();
            mi->setDoNotDisturb();
            mi->setEnableDisplayOnLiftWrist();
            mi->setRotateWristToSwitchInfo(true);
            mi->setInactivityWarnings();
            mi->setDisconnectNotification();
            mi->requestAlarms();
        }

        HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
        if (hrm) {
            hrm->setAllDayHRM();
            hrm-> setHeartrateSleepSupport();
        }

        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

