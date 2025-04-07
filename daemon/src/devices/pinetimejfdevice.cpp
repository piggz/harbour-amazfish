#include "pinetimejfdevice.h"
#include "deviceinfoservice.h"
#include "currenttimeservice.h"
#include "alertnotificationservice.h"
#include "pinetimemusicservice.h"
#include "infinitimefirmwareinfo.h"
#include "dfuservice.h"
#include "dfuoperation.h"
#include "infinitimenavservice.h"
#include "immediatealertservice.h"
#include "hrmservice.h"
#include "infinitimemotionservice.h"
#include "infinitimeweatherservice.h"
#include "pinetimesimpleweatherservice.h"
#include "adafruitblefsservice.h"
#include "batteryservice.h"
#include "amazfishconfig.h"
#include "realtimeactivitysample.h"
#include <QtXml/QtXml>

namespace {
size_t GetMtuForCharacteristic(QString path, const char* characteristicUUID)
{
    size_t transferMtu = 20;
    QDBusInterface adapterIntro("org.bluez", path, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> adapterXml = adapterIntro.call("Introspect");
    QDomDocument adapterDoc;
    adapterDoc.setContent(adapterXml.value());
    QDomNodeList adapterNodes = adapterDoc.elementsByTagName("node");

    for (int x = 0; x < adapterNodes.count(); x++)
    {
        QDomElement node = adapterNodes.at(x).toElement();
        QString nodeName = node.attribute("name");
        if (nodeName.startsWith("char")) {
            QString nodePath = path + "/" + nodeName;
            QDBusInterface characteristicInterface("org.bluez", nodePath, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
            QString currentCharacteristicUUID = characteristicInterface.property("UUID").toString();
            if(currentCharacteristicUUID == characteristicUUID){
                QDBusInterface mtuInterface("org.bluez", nodePath, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
                QString mtu = mtuInterface.property("MTU").toString();
                transferMtu = mtu.toInt();
            }

        }
    }
    return transferMtu;
}
}

PinetimeJFDevice::PinetimeJFDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &PinetimeJFDevice::onPropertiesChanged, Qt::UniqueConnection);
}

void PinetimeJFDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();

}

int PinetimeJFDevice::supportedFeatures() const
{
    return FEATURE_HRM |
            FEATURE_ALERT |
            FEATURE_WEATHER |
            FEATURE_STEPS;
}

QString PinetimeJFDevice::deviceType() const
{
    return "pinetimejf";
}

void PinetimeJFDevice::abortOperations()
{
    qDebug() << Q_FUNC_INFO;
    DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
    if (fw){
        fw->cancelOperation();
    }
}

void PinetimeJFDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        qDebug() << Q_FUNC_INFO << "Have an alert service";
        alert->sendAlert(sender, subject, message);
    }
}

void PinetimeJFDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->incomingCall(QByteArray::fromHex("030100"), caller);
    }
}

void PinetimeJFDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO << "not available";
}


void PinetimeJFDevice::parseServices()
{
    qDebug() << Q_FUNC_INFO;

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

            if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO  && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
                addService(DeviceInfoService::UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == CurrentTimeService::UUID_SERVICE_CURRENT_TIME  && !service(CurrentTimeService::UUID_SERVICE_CURRENT_TIME)) {
                addService(CurrentTimeService::UUID_SERVICE_CURRENT_TIME, new CurrentTimeService(path, this));
            } else if (uuid == AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION  && !service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION)) {
                addService(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == PineTimeMusicService::UUID_SERVICE_MUSIC  && !service(PineTimeMusicService::UUID_SERVICE_MUSIC  )) {
                addService(PineTimeMusicService::UUID_SERVICE_MUSIC  , new PineTimeMusicService(path, this));
            } else if (uuid == DfuService::UUID_SERVICE_DFU && !service(DfuService::UUID_SERVICE_DFU)) {
                addService(DfuService::UUID_SERVICE_DFU, new DfuService(path, this));
            } else if (uuid == InfiniTimeNavService::UUID_SERVICE_NAVIGATION && !service(InfiniTimeNavService::UUID_SERVICE_NAVIGATION)) {
                addService(InfiniTimeNavService::UUID_SERVICE_NAVIGATION, new InfiniTimeNavService(path, this));
            } else if (uuid == HRMService::UUID_SERVICE_HRM && !service(HRMService::UUID_SERVICE_HRM)) {
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == InfiniTimeMotionService::UUID_SERVICE_MOTION && !service(InfiniTimeMotionService::UUID_SERVICE_MOTION)) {
                addService(InfiniTimeMotionService::UUID_SERVICE_MOTION, new InfiniTimeMotionService(path, this));
            } else if (uuid == PineTimeSimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER && !service(PineTimeSimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER)) {
                addService(PineTimeSimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER, new PineTimeSimpleWeatherService(path, this));
            } else if (uuid == InfiniTimeWeatherService::UUID_SERVICE_WEATHER && !service(InfiniTimeWeatherService::UUID_SERVICE_WEATHER)) {
                addService(InfiniTimeWeatherService::UUID_SERVICE_WEATHER, new InfiniTimeWeatherService(path, this));
            } else if (uuid == AdafruitBleFsService::UUID_SERVICE_FS && !service(AdafruitBleFsService::UUID_SERVICE_FS)) {
                size_t transferMtu = GetMtuForCharacteristic(path, AdafruitBleFsService::UUID_CHARACTERISTIC_FS_TRANSFER);
                addService(AdafruitBleFsService::UUID_SERVICE_FS, new AdafruitBleFsService(path, this, transferMtu));
            } else if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
                addService(BatteryService::UUID_SERVICE_BATTERY, new BatteryService(path, this));
            } else if (uuid == ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT && !service(ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT)) {
                addService(ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT, new ImmediateAlertService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void PinetimeJFDevice::initialise()
{
    setConnectionState("connected");
    parseServices();

    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->enableNotification(AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT);
        connect(alert, &AlertNotificationService::serviceEvent, this, &PinetimeJFDevice::serviceEvent, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &PinetimeJFDevice::informationChanged, Qt::UniqueConnection);
    }

    BatteryService *battery = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (battery) {
        connect(battery, &BatteryService::informationChanged, this, &PinetimeJFDevice::informationChanged, Qt::UniqueConnection);
    }

    CurrentTimeService *cts = qobject_cast<CurrentTimeService*>(service(CurrentTimeService::UUID_SERVICE_CURRENT_TIME));
    if (cts) {
        cts->currentTime();
        cts->setCurrentTime();
    }

    PineTimeMusicService *ms = qobject_cast<PineTimeMusicService*>(service(PineTimeMusicService::UUID_SERVICE_MUSIC));
    if (ms) {
        ms->enableNotification(PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_EVENT);
        connect(ms, &PineTimeMusicService::serviceEvent, this, &PinetimeJFDevice::serviceEvent, Qt::UniqueConnection);
    }

    DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
    if (fw) {
        connect(fw, &DfuService::message, this, &PinetimeJFDevice::message, Qt::UniqueConnection);
        connect(fw, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        hrm->enableRealtimeHRMeasurement(AmazfishConfig::instance()->deviceRealtimeHRMMeasurement());

        connect(hrm, &HRMService::informationChanged, this, &AbstractDevice::informationChanged, Qt::UniqueConnection);
        connect(hrm, &HRMService::informationChanged, &realtimeActivitySample, &RealtimeActivitySample::slot_informationChanged, Qt::UniqueConnection);
    }

    InfiniTimeMotionService *motion = qobject_cast<InfiniTimeMotionService*>(service(InfiniTimeMotionService::UUID_SERVICE_MOTION));
    if (motion) {
        motion->enableNotification(InfiniTimeMotionService::UUID_CHARACTERISTIC_MOTION_STEPS);
        //motion->enableNotification(InfiniTimeMotionService::UUID_CHARACTERISTIC_MOTION_MOTION);
        connect(motion, &InfiniTimeMotionService::informationChanged, this, &AbstractDevice::informationChanged, Qt::UniqueConnection);
        connect(motion, &InfiniTimeMotionService::informationChanged, &realtimeActivitySample, &RealtimeActivitySample::slot_informationChanged, Qt::UniqueConnection);
    }

    connect(&realtimeActivitySample, &RealtimeActivitySample::samplesReady, this, &PinetimeJFDevice::sampledActivity, Qt::UniqueConnection);
}

void PinetimeJFDevice::sampledActivity(QDateTime dt, int kind, int intensity, int steps, int heartrate) {
    qDebug() << Q_FUNC_INFO << dt << kind << intensity << steps << heartrate;


    if (!m_conn || !(m_conn->isDatabaseUsed())) {
        qDebug() << Q_FUNC_INFO << "Database not connected";
        return;
    }

    auto config = AmazfishConfig::instance();
    uint id = qHash(config->profileName());
    uint devid = qHash(config->pairedAddress());

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto mibandActivity = m_conn->tableSchema("mi_band_activity");

    fields.addField(mibandActivity->field("timestamp"));
    fields.addField(mibandActivity->field("timestamp_dt"));
    fields.addField(mibandActivity->field("device_id"));
    fields.addField(mibandActivity->field("user_id"));
    fields.addField(mibandActivity->field("raw_intensity"));
    fields.addField(mibandActivity->field("steps"));
    fields.addField(mibandActivity->field("raw_kind"));
    fields.addField(mibandActivity->field("heartrate"));

    QList<QVariant> values;

    values << dt.toMSecsSinceEpoch() / 1000;
    values << dt;
    values << devid;
    values << id;
    values << intensity;
    values << steps;
    values << kind;
    values << heartrate;

    if (!m_conn->insertRecord(&fields, values)) {
        qWarning() << "error inserting record";
        tg.rollback();
        return;
    }
    tg.commit();

}


void PinetimeJFDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        m_reconnectTimer->start();
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
    }

}

void PinetimeJFDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

AbstractFirmwareInfo *PinetimeJFDevice::firmwareInfo(const QByteArray &bytes)
{
    return new InfinitimeFirmwareInfo(bytes);
}

void PinetimeJFDevice::navigationRunning(bool running)
{

}

void PinetimeJFDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    qDebug() << Q_FUNC_INFO;
    InfiniTimeNavService *nav = qobject_cast<InfiniTimeNavService*>(service(InfiniTimeNavService::UUID_SERVICE_NAVIGATION));
    if (nav) {
        nav->setFlag(flag);
        nav->setNarrative(narrative);
        nav->setManDist(manDist);
        nav->setProgress(progress);
    }
}

void PinetimeJFDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration, int position)
{
    PineTimeMusicService *music = qobject_cast<PineTimeMusicService*>(service(PineTimeMusicService::UUID_SERVICE_MUSIC));
    if (music) {
        music->setStatus(playing);
        music->setAlbum(album);
        music->setTrack(title);
        music->setArtist(artist);
    }
}

void PinetimeJFDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    firmwareType = info->type();
    if(info->type() == AbstractFirmwareInfo::Type::Firmware) {
        DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
        if (fw){
            if (fw->currentOperation()) {
                emit message(tr("An operation is currently running, please try later"));
                return;
            }
            DfuOperation *dfu = new DfuOperation(info, fw, this);
            if (fw->registerOperation(dfu)) {
                emit operationRunningChanged();
            } else {
                delete dfu;
            }
        }
    } else if (info->type() == AbstractFirmwareInfo::Type::Res_Compressed) {
        AdafruitBleFsService* s = qobject_cast<AdafruitBleFsService*>(service(AdafruitBleFsService::UUID_SERVICE_FS));
        if(s) {
            if (s->currentOperation()) {
                emit message(tr("An operation is currently running, please try later"));
                return;
            }
            AdafruitBleFsOperation *updateFirmwareOperation =  new AdafruitBleFsOperation(s, info, this);
            if (s->registerOperation(updateFirmwareOperation)) {
                emit operationRunningChanged();
            } else {
                delete updateFirmwareOperation;
            }
        }
    }
}

void PinetimeJFDevice::startDownload()
{
    qDebug() << Q_FUNC_INFO;

    if(firmwareType == AbstractFirmwareInfo::Type::Firmware)
    {
        DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
        if (fw) {
            DfuOperation *operation = dynamic_cast<DfuOperation*>(fw->currentOperation());
            if (operation) {
                emit message(tr("Sending file..."));
                operation->start(fw);
            } else {
                emit message(tr("No file selected"));
            }
        }
    }
    else if(firmwareType == AbstractFirmwareInfo::Type::Res_Compressed)
    {
        AdafruitBleFsService* s = qobject_cast<AdafruitBleFsService*>(service(AdafruitBleFsService::UUID_SERVICE_FS));
        if(s) {
            AdafruitBleFsOperation *operation = dynamic_cast<AdafruitBleFsOperation*>(s->currentOperation());
            if (operation) {
                emit message(tr("Sending file..."));
                operation->start(s);
            } else {
                emit message(tr("No file selected"));
            }
        }
    }
}

void PinetimeJFDevice::refreshInformation()
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }

    BatteryService *bat = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (bat) {
        bat->refreshInformation();
    }

    InfiniTimeMotionService *motion = qobject_cast<InfiniTimeMotionService*>(service(InfiniTimeMotionService::UUID_SERVICE_MOTION));
    if (motion) {
        motion->refreshSteps();
        //        motion->refreshMotion();
    }

}

QString PinetimeJFDevice::information(Info i) const
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (!info) {
        qWarning() << "Device info service doesn't exists";
        return QString();
    }

    InfiniTimeMotionService *motion = qobject_cast<InfiniTimeMotionService*>(service(InfiniTimeMotionService::UUID_SERVICE_MOTION));
    if (!motion) {
        qWarning() << "Motion service doesn't exists";
        return QString();
    }

    switch(i) {
    case INFO_MODEL:
        return info->modelNumber();
        break;
    case INFO_SERIAL:
        return info->serialNumber();
        break;
    case INFO_FW_REVISION:
        return info->fwRevision();
        break;
    case INFO_HWVER:
        return info->hardwareRevision();
        break;
    case INFO_MANUFACTURER:
        return info->manufacturerName();
        break;
    case INFO_STEPS:
        return QString::number(motion->steps());
        break;
    }
    return QString();
}

void PinetimeJFDevice::serviceEvent(const QString &characteristic, uint8_t event)
{
    qDebug() << Q_FUNC_INFO << characteristic << event;

    if (characteristic == PineTimeMusicService::UUID_CHARACTERISTIC_MUSIC_EVENT) {
        switch(event) {
        case PineTimeMusicService::EVENT_MUSIC_PLAY:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PLAY);
            break;
        case PineTimeMusicService::EVENT_MUSIC_PAUSE:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PAUSE);
            break;
        case PineTimeMusicService::EVENT_MUSIC_NEXT:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_NEXT);
            break;
        case PineTimeMusicService::EVENT_MUSIC_PREV:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_PREV);
            break;
        case PineTimeMusicService::EVENT_MUSIC_VOLUP:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLUP);
            break;
        case PineTimeMusicService::EVENT_MUSIC_VOLDOWN:
            emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLDOWN);
            break;
        case PineTimeMusicService::EVENT_MUSIC_OPEN:
            emit deviceEvent(AbstractDevice::EVENT_APP_MUSIC);
            break;

        default:
            break;
        }
    } else if (characteristic == AlertNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT) {
        switch(event) {
        case AlertNotificationService::CALL_REJECT:
            emit deviceEvent(AbstractDevice::EVENT_DECLINE_CALL);
            break;
        case AlertNotificationService::CALL_ANSWER:
            emit deviceEvent(AbstractDevice::EVENT_ANSWER_CALL);
            break;
        case AlertNotificationService::CALL_IGNORE:
            emit deviceEvent(AbstractDevice::EVENT_IGNORE_CALL);
            break;
        }
    }
}

void PinetimeJFDevice::sendWeather(CurrentWeather *weather)
{
    InfiniTimeWeatherService *w = qobject_cast<InfiniTimeWeatherService*>(service(InfiniTimeWeatherService::UUID_SERVICE_WEATHER));
    if (w){
        w->sendWeather(weather);
    }

    PineTimeSimpleWeatherService *sw = qobject_cast<PineTimeSimpleWeatherService*>(service(PineTimeSimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER));

    if (sw){
        sw->sendWeather(weather);
    }
}

void PinetimeJFDevice::applyDeviceSetting(Settings s)
{

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    switch(s) {
    case SETTING_DEVICE_REALTIME_HRM_MEASUREMENT:
        if (hrm) {
            hrm->enableRealtimeHRMeasurement(AmazfishConfig::instance()->deviceRealtimeHRMMeasurement());
        }
        break;
    }

}

void PinetimeJFDevice::immediateAlert(int level)
{
    ImmediateAlertService *ias = qobject_cast<ImmediateAlertService*>(service(ImmediateAlertService::UUID_SERVICE_IMMEDIATE_ALERT));
    if (ias) {
        ias->sendAlert((ImmediateAlertService::Levels)level);
    }
}
