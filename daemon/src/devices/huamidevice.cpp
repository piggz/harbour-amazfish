#include "huamidevice.h"
#include "bipfirmwareinfo.h"
#include "updatefirmwareoperation.h"
#include "immediatealertservice.h"
#include "amazfishconfig.h"

#include <QtXml/QtXml>

HuamiDevice::HuamiDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_keyPressTimer = new QTimer(this);
    m_keyPressTimer->setInterval(500);
    connect(this, &QBLEDevice::propertiesChanged, this, &HuamiDevice::onPropertiesChanged);
    connect(m_keyPressTimer, &QTimer::timeout, this, &HuamiDevice::buttonPressTimeout);
}

void HuamiDevice::sendWeatherHuami(CurrentWeather *weather, bool sendConditionString)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->sendWeather(weather, sendConditionString);
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

void HuamiDevice::downloadSportsData()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->fetchSportsSummaries();
    }
}

void HuamiDevice::downloadActivityData()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->fetchActivityData();
    }
}

void HuamiDevice::fetchLogs()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->fetchLogs();
    }
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
    qDebug() << Q_FUNC_INFO << caller;
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->incomingCall(QByteArray::fromHex("0301"), caller);
    }
}

void HuamiDevice::incomingCallEnded()
{
    ImmediateAlertService *ias = qobject_cast<ImmediateAlertService*>(service(ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT));
    if (ias) {
        ias->sendAlert(ImmediateAlertService::Levels::NoAlert);
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

void HuamiDevice::setDatabase(KDbConnection *conn)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->setDatabase(conn);
    }
    AbstractDevice::setDatabase(conn);
}

void HuamiDevice::handleButtonPressed()
{
    qDebug() << Q_FUNC_INFO;
    m_buttonPresses++;
    m_keyPressTimer->stop();
    m_keyPressTimer->start();
}

void HuamiDevice::buttonPressTimeout()
{
    qDebug() << Q_FUNC_INFO;
    int presses = m_buttonPresses;
    m_buttonPresses = 0;
    m_keyPressTimer->stop();
    emit buttonPressed(presses);
}


void HuamiDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        m_needsAuth = false;

        MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
        if (mi){
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

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
            applyDeviceSetting(SETTING_DEVICE_DISPLAY_ITEMS);
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


void HuamiDevice::applyDeviceSetting(Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (!hrm) {
        return;
    }
    switch(s) {
    case SETTING_ALARMS:
        mi->setAlarms();
        break;
    case SETTING_DEVICE_DATE:
        mi->setDateDisplay();
        break;
    case SETTING_DEVICE_DISPLAY_ITEMS:
        mi->setDisplayItems();
        break;
    case SETTING_DEVICE_LANGUAGE:
        mi->setLanguage();
        break;
    case SETTING_DEVICE_TIME:
        mi->setCurrentTime();
        break;
    case SETTING_DEVICE_UNIT:
        mi->setDistanceUnit();
        break;
    case SETTING_USER_ALERT_GOAL:
        mi->setAlertFitnessGoal();
        break;
    case SETTING_USER_ALL_DAY_HRM:
        hrm->setAllDayHRM();
        break;
    case SETTING_USER_DISPLAY_ON_LIFT:
        mi->setEnableDisplayOnLiftWrist();
        break;
    case SETTING_USER_GOAL:
        mi->setFitnessGoal();
        break;
    case SETTING_USER_HRM_SLEEP_DETECTION:
        hrm->setHeartrateSleepSupport();
        break;
    case SETTING_USER_PROFILE:
        mi->setUserInfo();
        break;
    case SETTING_DISCONNECT_NOTIFICATION:
        mi->setDisconnectNotification();
        break;
    case SETTING_DEVICE_REALTIME_HRM_MEASUREMENT:
        hrm->enableRealtimeHRMeasurement(AmazfishConfig::instance()->deviceRealtimeHRMMeasurement());
        break;
    }
}

void HuamiDevice::startDownload()
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->startDownload();
    }
}

void HuamiDevice::abortOperations()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->abortOperations();
    }
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->abortOperations();
    }
}

void HuamiDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->prepareFirmwareDownload(info, new UpdateFirmwareOperation(info, fw));
    }
}

void HuamiDevice::serviceEvent(char event)
{
    switch(event) {
    case MiBandService::EVENT_DECLINE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_DECLINE_CALL);
        break;
    case MiBandService::EVENT_IGNORE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_IGNORE_CALL);
        break;
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

void HuamiDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        m_reconnectTimer->start();
        if (map.contains("Paired")) {
            bool value = map["Paired"].toBool();

            if (value) {
                setConnectionState("paired");
            }
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
        }
    }
}
