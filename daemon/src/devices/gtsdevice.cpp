#include "gtsdevice.h"
#include "gtsfirmwareinfo.h"
#include <QtXml/QtXml>
#include <QDateTime>
#include "typeconversion.h"
#include "updatefirmwareoperationnew.h"
#include "huamiupdatefirmwareoperation2020.h"
#include "amazfishconfig.h"

GtsDevice::GtsDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << "Creating GTS Device";
    connect(this, &QBLEDevice::propertiesChanged, this, &GtsDevice::onPropertiesChanged);
}

QString GtsDevice::deviceType()
{
    return "amazfitgts";
}

int GtsDevice::supportedFeatures()
{
    return FEATURE_HRM |
            FEATURE_WEATHER |
            FEATURE_ACTIVITY |
            FEATURE_STEPS |
            FEATURE_ALARMS |
            FEATURE_ALERT |
            FEATURE_EVENT_REMINDER |
            FEATURE_MUSIC_CONTROL |
            FEATURE_BUTTON_ACTION;
}

void GtsDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->sendAlert(sender, subject, message);
    }
}

void GtsDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << "GtsDevice::onPropertiesChanged:" << interface << map << list;

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
        } else if (map.contains("Paired")) {
            bool value = map["Paired"].toBool();

            if (value) {
                qDebug() << "Paired!";
                if (m_connectionState == "pairing" && m_pairing) {
                    connectToDevice();
                }
            }
        }
    }

}

void GtsDevice::serviceEvent(uint8_t event)
{
    switch(event) {
    case MiBandService::EVENT_MUSIC_PLAY:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PLAY);
        break;
    case MiBandService::EVENT_MUSIC_PAUSE:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PAUSE);
        break;
    case MiBandService::EVENT_MUSIC_NEXT:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_NEXT);
        break;
    case MiBandService::EVENT_MUSIC_PREV:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_PREV);
        break;
    case MiBandService::EVENT_MUSIC_VOLUP:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLUP);
        break;
    case MiBandService::EVENT_MUSIC_VOLDOWN:
        emit deviceEvent(AbstractDevice::EVENT_MUSIC_VOLDOWN);
        break;
    case MiBandService::EVENT_MUSIC_OPEN:
        emit deviceEvent(AbstractDevice::EVENT_APP_MUSIC);
        break;
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

void GtsDevice::initialise()
{
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &GtsDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &GtsDevice::serviceEvent, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got mi2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(false);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(fw, &BipFirmwareService::downloadProgress, this, &HuamiDevice::downloadProgress, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
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
    if (revision > "0.0.9.0") {
        qDebug() << "GTS with new FW";
        m_ActivitySampleSize = 8;
    }
}


void GtsDevice::parseServices()
{
    qDebug() << "GtsDevice::parseServices";

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), nullptr);
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

            QDBusInterface devInterface("org.bluez", path, "org.bluez.GattService1", QDBusConnection::systemBus(), nullptr);
            QString uuid = devInterface.property("UUID").toString();

            qDebug() << "Creating service for: " << uuid;

            if (uuid == AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION && !service(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION)) {
                addService(AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == DeviceInfoService::UUID_SERVICE_DEVICEINFO  && !service(DeviceInfoService::UUID_SERVICE_DEVICEINFO)) {
                addService(DeviceInfoService::UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == HRMService::UUID_SERVICE_HRM && !service(HRMService::UUID_SERVICE_HRM)) {
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == MiBandService::UUID_SERVICE_MIBAND && !service(MiBandService::UUID_SERVICE_MIBAND)) {
                addService(MiBandService::UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == MiBand2Service::UUID_SERVICE_MIBAND2 && !service(MiBand2Service::UUID_SERVICE_MIBAND2)) {
                addService(MiBand2Service::UUID_SERVICE_MIBAND2, new MiBand2Service(path, 0x00, 0x80, true, this));
            } else if (uuid == BipFirmwareService::UUID_SERVICE_FIRMWARE && !service(BipFirmwareService::UUID_SERVICE_FIRMWARE)) {
                addService(BipFirmwareService::UUID_SERVICE_FIRMWARE, new BipFirmwareService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}



AbstractFirmwareInfo *GtsDevice::firmwareInfo(const QByteArray &bytes)
{
    return new GtsFirmwareInfo(bytes);
}

void GtsDevice::sendEventReminder(int id, const QDateTime &dt, const QString &event)
{
    //Send event reminder
    //Type: 02
    //00 0b Always 0b
    //01 01 ID
    //02 09 Flags 0x01 = Enable, 0x04 = End Date Preset, 0x08 = Text Present
    //03 00
    //04 00
    //05 00
    //06 Date/Time (6)
    //00
    //MESSAGE
    qDebug() << dt << event;

    QByteArray cmd;
    cmd += (char)0x0b;
    cmd += (char)id;
    cmd += (char)0x09;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += TypeConversion::dateTimeToBytes(dt, 0).left(5);
    cmd += (char)0x00;
    cmd += (char)0x00;
    cmd += event.toLocal8Bit();
    cmd += (char)0x00;

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 2, cmd);
    }
}

void GtsDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw){
        QString revision = softwareRevision();
        if (revision > "0.1.1.16") {
            fw->prepareFirmwareDownload(info, new HuamiUpdateFirmwareOperation2020(info, fw));
        } else {
            fw->prepareFirmwareDownload(info, new UpdateFirmwareOperationNew(info, fw));
        }
    }
}

void GtsDevice::applyDeviceSetting(AbstractDevice::Settings s)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (!mi) {
        return;
    }

    if (s == SETTING_DEVICE_DISPLAY_ITEMS) {
        mi->setDisplayItemsNew();
    } else {
        HuamiDevice::applyDeviceSetting(s);
    }
}

void GtsDevice::sendWeather(CurrentWeather *weather)
{
    sendWeatherHuami(weather, true);
}

void GtsDevice::enableFeature(AbstractDevice::Feature feature)
{
    qDebug() << Q_FUNC_INFO << feature;
    if (feature == AbstractDevice::FEATURE_MUSIC_CONTROL) {
        QByteArray cmd;

        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x01;
        cmd += (char)0x00;
        MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
        if (mi){
            mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, cmd);
        }

        QString track = "Test track";
        QString album = "Awesome Album";
        QString artist = "An Artist";

        char flags = 0x00;
        flags |= 0x01;

        flags |= 0x02;
        flags |= 0x04;
        flags |= 0x08;

        char state = 0x00; //Not playing
        cmd.clear();
        cmd += flags;
        cmd += state;

        //Unknown
        cmd += (char)0x01;
        cmd += (char)0x00;
        cmd += (char)0x00;
        cmd += (char)0x00;

        //Show Track
        cmd += (char)0x01;
        cmd += (char)0x00;

        cmd += track.toLocal8Bit();
        cmd += char(0x00);
        cmd += album.toLocal8Bit();
        cmd += char(0x00);
        cmd += artist.toLocal8Bit();
        cmd += char(0x00);

        if (mi){
            mi->writeChunked(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 3, cmd);
        }
    }
}

void GtsDevice::setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration, int position)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi){
        mi->setMusicStatus(playing, artist, album, track, duration, position);
    }
}

void GtsDevice::navigationRunning(bool running)
{
    m_navigationRunning = running;
    if (m_navigationRunning) {
        setMusicStatus(m_navigationRunning,"", "", "");
    }
}

void GtsDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    Q_UNUSED(flag)
    setMusicStatus(m_navigationRunning, narrative, "", manDist, 1000, progress * 10);
}

QStringList GtsDevice::supportedDisplayItems() const
{
    QStringList items;

    items << "status";
    items << "pai";
    items << "hr";
    items << "workout";
    items << "activity";
    items << "weather";
    items << "music";
    items << "notifications";
    items << "alarm";
    items << "eventreminder";
    items << "more";
    items << "settings";

    return items;
}
