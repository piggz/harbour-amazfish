#include "pinetimejfdevice.h"
#include "deviceinfoservice.h"
#include "currenttimeservice.h"
#include "alertnotificationservice.h"
#include "pinetimemusicservice.h"
#include "infinitimefirmwareinfo.h"
#include "dfuservice.h"
#include "dfuoperation.h"
#include "infinitimenavservice.h"
#include "hrmservice.h"

#include <QtXml/QtXml>

PinetimeJFDevice::PinetimeJFDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << "PinetimeJFDevice:: " << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &PinetimeJFDevice::onPropertiesChanged, Qt::UniqueConnection);
}

QString PinetimeJFDevice::pair()
{
    qDebug() << "AbstractDevice::pair";

    m_needsAuth = false;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::connectToDevice();
    return "pairing";
}

int PinetimeJFDevice::supportedFeatures()
{
    return FEATURE_HRM |
            FEATURE_ALERT;
}

QString PinetimeJFDevice::deviceType()
{
    return "pinetimejf";
}

bool PinetimeJFDevice::operationRunning()
{
    return QBLEDevice::operationRunning();
}

void PinetimeJFDevice::abortOperations()
{
    qDebug() << Q_FUNC_INFO;
    DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
    if (fw){
        fw->abortOperations();
    }
}

void PinetimeJFDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        qDebug() << "PT Have an alert service";
        alert->sendAlert(sender, subject, message);
    }
}

void PinetimeJFDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
    AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION));
    if (alert) {
        alert->incomingCall(caller);
    }
}

void PinetimeJFDevice::parseServices()
{
    qDebug() << "PinetimeJFDevice::parseServices";

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
                addService(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this, 0x0a));
            } else if (uuid == PineTimeMusicService::UUID_SERVICE_MUSIC  && !service(PineTimeMusicService::UUID_SERVICE_MUSIC  )) {
                addService(PineTimeMusicService::UUID_SERVICE_MUSIC  , new PineTimeMusicService(path, this));
            } else if (uuid == DfuService::UUID_SERVICE_DFU && !service(DfuService::UUID_SERVICE_DFU)) {
                addService(DfuService::UUID_SERVICE_DFU, new DfuService(path, this));
            } else if (uuid == InfiniTimeNavService::UUID_SERVICE_NAVIGATION && !service(InfiniTimeNavService::UUID_SERVICE_NAVIGATION)) {
                addService(InfiniTimeNavService::UUID_SERVICE_NAVIGATION, new InfiniTimeNavService(path, this));
            } else if (uuid == HRMService::UUID_SERVICE_HRM && !service(HRMService::UUID_SERVICE_HRM)) {
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
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
        connect(fw, &DfuService::downloadProgress, this, &PinetimeJFDevice::downloadProgress, Qt::UniqueConnection);
        connect(fw, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &AbstractDevice::informationChanged, Qt::UniqueConnection);
    }
}

void PinetimeJFDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << "PinetimeJFDevice::onPropertiesChanged:" << interface << map << list;

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
    }

}

void PinetimeJFDevice::authenticated(bool ready)
{
    qDebug() << "PinetimeJFDevice::authenticated:" << ready;

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
    DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
    if (fw){
        fw->prepareFirmwareDownload(info, new DfuOperation(info, fw));
    }
}

void PinetimeJFDevice::startDownload()
{
    qDebug() << Q_FUNC_INFO;
    DfuService *fw = qobject_cast<DfuService*>(service(DfuService::UUID_SERVICE_DFU));
    if (fw){
        fw->startDownload();
    }
}

void PinetimeJFDevice::refreshInformation()
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }
}

QString PinetimeJFDevice::information(Info i) const
{
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (!info) {
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
