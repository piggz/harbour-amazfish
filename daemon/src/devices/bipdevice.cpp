#include "bipdevice.h"
#include "bipfirmwareinfo.h"
#include "updatefirmwareoperation.h"

#include <QtXml/QtXml>

const char* BipDevice::UUID_SERVICE_ALERT_NOTIFICATION = AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION;
const char* BipDevice::UUID_SERVICE_DEVICEINFO = DeviceInfoService::UUID_SERVICE_DEVICEINFO;
const char* BipDevice::UUID_SERVICE_HRM = HRMService::UUID_SERVICE_HRM;
const char* BipDevice::UUID_SERVICE_MIBAND = MiBandService::UUID_SERVICE_MIBAND;
const char* BipDevice::UUID_SERVICE_MIBAND2 = MiBand2Service::UUID_SERVICE_MIBAND2;
const char* BipDevice::UUID_SERVICE_FIRMWARE = BipFirmwareService::UUID_SERVICE_FIRMWARE;

BipDevice::BipDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    connect(this, &QBLEDevice::propertiesChanged, this, &BipDevice::onPropertiesChanged);

    m_keyPressTimer = new QTimer(this);
    m_keyPressTimer->setInterval(500);
    connect(m_keyPressTimer, &QTimer::timeout, this, &BipDevice::buttonPressTimeout);
}


int BipDevice::supportedFeatures()
{
    return FEATURE_HRM |
            FEATURE_WEATHER |
            FEATURE_ACTIVITY |
            FEATURE_STEPS |
            FEATURE_ALARMS |
            FEATURE_ALERT |
            FEATURE_NOTIFIATION;
}

QString BipDevice::deviceType()
{
    return "amazfitbip";
}

bool BipDevice::operationRunning()
{
    return QBLEDevice::operationRunning();
}
    
void BipDevice::parseServices()
{
    qDebug() << "BipDevice::parseServices";

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    qDebug() << "Resolved services...";

    qDebug().noquote() << xml.value();

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    qDebug() << nodes.count() << "nodes";

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("service")) {
            QString path = devicePath() + "/" + nodeName;

            QDBusInterface devInterface("org.bluez", path, "org.bluez.GattService1", QDBusConnection::systemBus(), 0);
            QString uuid = devInterface.property("UUID").toString();

            qDebug() << "Creating service for: " << uuid;

            if (uuid == UUID_SERVICE_ALERT_NOTIFICATION && !service(UUID_SERVICE_ALERT_NOTIFICATION)) {
                addService(UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == UUID_SERVICE_DEVICEINFO  && !service(UUID_SERVICE_DEVICEINFO)) {
                addService(UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == UUID_SERVICE_HRM && !service(UUID_SERVICE_HRM)) {
                addService(UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND && !service(UUID_SERVICE_MIBAND)) {
                addService(UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND2 && !service(UUID_SERVICE_MIBAND2)) {
                addService(UUID_SERVICE_MIBAND2, new MiBand2Service(path, 0x08, 0x00, false, this));
            } else if (uuid == UUID_SERVICE_FIRMWARE && !service(UUID_SERVICE_FIRMWARE)) {
                addService(UUID_SERVICE_FIRMWARE, new BipFirmwareService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void BipDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << "BipDevice::onPropertiesChanged:" << interface << map << list;

    if (interface == "org.bluez.Device1") {
        m_reconnectTimer->start();
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                qDebug() << "DisConnected!";
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }

#if 0
        if (map.contains("Paired")) {
            bool value = map["Paired"].toBool();

            if (value) {
                qDebug() << "Paired!";
                if (m_connectionState == "pairing" && m_pairing) {
                    connectToDevice();
                }
            }
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (value) {
                qDebug() << "Connected!";
                //Check if services are resolved
                bool resolved = deviceProperty("ServicesResolved").toBool();
                qDebug() << "Resolved:" << resolved;

                if (resolved) {
                    initialise();
                }
            }
        }
        if (map.contains("ServicesResolved")) {
            bool value = map["ServicesResolved"].toBool();

            if (value && !m_pairing) {
                qDebug() << "ServicesResolved!";
                initialise();
            }
        }
#endif
    }

}

void BipDevice::authenticated(bool ready)
{
    qDebug() << "BipInterface::authenticated:" << ready;

    if (ready) {
        m_needsAuth = false;

        MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
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

        HRMService *hrm = qobject_cast<HRMService*>(service(UUID_SERVICE_HRM));
        if (hrm) {
            hrm->setAllDayHRM();
            hrm-> setHeartrateSleepSupport();
        }

        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

void BipDevice::sendWeather(CurrentWeather *weather)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (mi){
        bool supportsConditionString = (softwareRevision() > "V0.0.8.74");
        mi->sendWeather(weather, supportsConditionString);
    }
}

AbstractFirmwareInfo *BipDevice::firmwareInfo(const QByteArray &bytes)
{
    return new BipFirmwareInfo(bytes);
}

void BipDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->prepareFirmwareDownload(info, new UpdateFirmwareOperation(info, fw));
    }
}

void BipDevice::startDownload()
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->startDownload();
    }
}

void BipDevice::abortOperations()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (mi){
        mi->abortOperations();
    }
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (fw){
        fw->abortOperations();
    }
}

void BipDevice::initialise()
{
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(mi, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &BipDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &BipDevice::serviceEvent, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got mi2 service" << m_pairing << m_needsAuth;
        connect(mi2, &MiBand2Service::authenticated, this, &BipDevice::authenticated, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(m_needsAuth);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(fw, &BipFirmwareService::downloadProgress, this, &BipDevice::downloadProgress, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }
}

void BipDevice::serviceEvent(char event)
{
    switch(event) {
    case MiBandService::EVENT_DECLINE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_DECLINE_CALL);
        break;
    case MiBandService::EVENT_IGNORE_CALL:
        emit deviceEvent(AbstractDevice::EVENT_IGNORE_CALL);
        break;
    default:
        break;
    }
}

QString BipDevice::softwareRevision()
{
    if (m_softwareRevision.isEmpty()) {
        DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(UUID_SERVICE_DEVICEINFO));
        if (info) {
            m_softwareRevision = info->readSoftwareRevisionSync();
        } else {
            m_softwareRevision = "V0.0.0.00";
        }
    }
    return m_softwareRevision;
}

void BipDevice::handleButtonPressed()
{
    m_buttonPresses++;
    m_keyPressTimer->stop();
    m_keyPressTimer->start();
}

void BipDevice::buttonPressTimeout()
{
    int presses = m_buttonPresses;
    m_buttonPresses = 0;
    m_keyPressTimer->stop();
    emit buttonPressed(presses);
}

void BipDevice::refreshInformation()
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(UUID_SERVICE_DEVICEINFO));
    if (info) {
         info->refreshInformation();
    }
    
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (mi) {
        mi->requestGPSVersion();
        mi->requestBatteryInfo();
    }
}

QString BipDevice::information(Info i)
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(UUID_SERVICE_DEVICEINFO));
     if (!info) {
        return QString();
    }
    
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
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

void BipDevice::applyDeviceSetting(Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(UUID_SERVICE_HRM));
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
    }

}

void BipDevice::stepsChanged()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    emit informationChanged(AbstractDevice::INFO_STEPS, QString::number(mi->steps()));
}

void BipDevice::batteryInfoChanged()
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    emit informationChanged(AbstractDevice::INFO_BATTERY, QString::number(mi->batteryInfo()));
}

void BipDevice::rebootWatch()
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (!fw) {
        return;
    }
    fw->writeValue(BipFirmwareService::UUID_CHARACTERISTIC_FIRMWARE, QByteArray(1, BipFirmwareService::COMMAND_FIRMWARE_REBOOT));

}

void BipDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->sendAlert(sender, subject, message);
    }
}

void BipDevice::incomingCall(const QString &caller)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->incomingCall(caller);
    }
}
