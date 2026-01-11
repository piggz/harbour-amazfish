#include "huamidevice.h"
#include "logfetchoperation.h"
#include "activityfetchoperation.h"
#include "sportssummaryoperation.h"
#include "sportsdetailoperation.h"
#include "updatefirmwareoperation.h"
#include "immediatealertservice.h"
#include "amazfishconfig.h"
#include "huamiactivitysummaryparser.h"
#include "bipactivitydetailparser.h"

#include <QtXml/QtXml>

HuamiDevice::HuamiDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;
    m_keyPressTimer = new QTimer(this);
    m_keyPressTimer->setInterval(500);

    m_fetcher = new HuamiFetcher(this);

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
    qDebug() << Q_FUNC_INFO;
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        SportsSummaryOperation *sportsSummaryOperation = new SportsSummaryOperation(mi, m_conn, true, activitySummaryParser());
        //if (mi->registerOperation(sportsSummaryOperation)) {
        //    sportsSummaryOperation->start(mi);
        //    emit operationRunningChanged();
        //} else {
        //    delete sportsSummaryOperation;
        //}
    }
}

void HuamiDevice::downloadActivityData()
{
    /*
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        int sampleSize = activitySampleSize();
        ActivityFetchOperation *activityFetchOperation = new ActivityFetchOperation(mi, m_conn, sampleSize, true);
        if (mi->registerOperation(activityFetchOperation)) {
            activityFetchOperation->start(mi);
            emit operationRunningChanged();
        } else {
            delete activityFetchOperation;
        }
    }*/
    m_fetcher->startFetchData(Amazfish::DataType::TYPE_ACTIVITY);
}

void HuamiDevice::fetchLogs()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));

    if (mi){
        LogFetchOperation *logFetchOperation = new LogFetchOperation();
        //if (mi->registerOperation(logFetchOperation)) {
        //    logFetchOperation->start(mi);
        //    emit operationRunningChanged();
        //} else {
        //    delete logFetchOperation;
        //}
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

void HuamiDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->sendAlert(notification.appName, notification.summary, notification.body);
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
    Amazfish::WatchNotification n;
    n.id = 0;
    n.appId = "uk.co.piggz.amazfish.navigation";
    n.appName = "navigation";
    n.summary = msg;
    n.body = "";

    sendAlert(n);
}

void HuamiDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    Q_UNUSED(flag)

    Amazfish::WatchNotification n;
    n.id = 0;
    n.appId = "uk.co.piggz.amazfish.navigation";
    n.appName = "navigation";
    n.summary = tr("Progress") + ":" + QString::number(progress);
    n.body = narrative + "\n" + manDist;

    sendAlert(n);
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
        UpdateFirmwareOperation *operation = dynamic_cast<UpdateFirmwareOperation*>(fw->currentOperation());
        if (operation) {
            emit message(tr("Sending %1...").arg(operation->version()));
            operation->start(fw);
        } else {
            emit message(tr("No file selected"));
        }
    }
}

void HuamiDevice::abortOperations()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        //mi->cancelOperation();
    }
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->cancelOperation();
    }
}

void HuamiDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        if (fw->currentOperation()) {
            emit message(tr("An operation is currently running, please try later"));
            return;
        }
        UpdateFirmwareOperation *updateFirmwareOperation =  new UpdateFirmwareOperation(info, fw, this);
        if (fw->registerOperation(updateFirmwareOperation)) {
            emit operationRunningChanged();
        } else {
            delete updateFirmwareOperation;
        }
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

void HuamiDevice::operationComplete(AbstractOperation *operation)
{
    qDebug() << Q_FUNC_INFO;
    SportsSummaryOperation *sportSummary = dynamic_cast<SportsSummaryOperation*>(operation);

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));

    if (sportSummary && mi) {
        //Now the summary is finished, need to get the detail
        bool createDetail = false;
        ActivitySummary summary;
        if (sportSummary->success()) {
            qDebug() << "Finished summary data, now getting track detail";
            summary = sportSummary->summary();
            createDetail = true;
        }

        if (createDetail) {
            SportsDetailOperation *sportsDetailOperation = new SportsDetailOperation(mi, m_conn, summary, true, activityDetailParser());
            //if (mi->registerOperation(sportsDetailOperation)) {
            //    sportsDetailOperation->start(mi);
            //    emit operationRunningChanged();
            //} else {
            //    delete sportsDetailOperation;
            //}
        }
    }
}

AbstractActivitySummaryParser *HuamiDevice::activitySummaryParser() const
{
    return new HuamiActivitySummaryParser();
}

AbstractActivityDetailParser *HuamiDevice::activityDetailParser() const
{
    return new BipActivityDetailParser();
}

void HuamiDevice::setActivityNotifications(bool control, bool data)
{
    qDebug() << Q_FUNC_INFO << control << data;
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        if (control) {
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_CONTROL);
        } else {
            mi->disableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_CONTROL);
        }
        if (data) {
            mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
        } else {
            mi->disableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
        }
    }
}

void HuamiDevice::writeActivityControl(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_CONTROL, value);
    }
}

void HuamiDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
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
            int elapsed = init_dt.secsTo(QDateTime::currentDateTime());
            qDebug() << "initialise() elapsed: " << elapsed << "starting: " << (elapsed >60);
            if (elapsed > 60) {
                init_dt = QDateTime::currentDateTime();
                initialise();
            }
        }
    }
}
