#include <QtXml/QtXml>

#include "deviceinfoservice.h"
#include "pebbleservice.h"
#include "pebbledevice.h"

PebbleDevice::PebbleDevice(const QString &pairedName, QObject *parent ) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &PebbleDevice::onPropertiesChanged, Qt::UniqueConnection);
}

int PebbleDevice::supportedFeatures() const
{
    //  FEATURE_HRM | FEATURE_WEATHER | FEATURE_ACTIVITY |FEATURE_STEPS | FEATURE_ALARMS | FEATURE_ALERT | FEATURE_EVENT_REMINDER | FEATURE_MUSIC_CONTROL | FEATURE_BUTTON_ACTION | FEATURE_SCREENSHOT
    return 0;
}
QString PebbleDevice::deviceType() const 
{
    return "pebble";
}

void PebbleDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
}


void PebbleDevice::sendAlert(const Amazfish::WatchNotification &notification) {
    qDebug() << Q_FUNC_INFO;
}

void PebbleDevice::incomingCall(const QString &caller) {
    qDebug() << Q_FUNC_INFO;
}

void PebbleDevice::incomingCallEnded() {
    qDebug() << Q_FUNC_INFO;
}

AbstractFirmwareInfo* PebbleDevice::firmwareInfo(const QByteArray &bytes) {
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

//    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;
//    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;
//    virtual void requestScreenshot() override;


void PebbleDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
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

void PebbleDevice::parseServices()
{

    qDebug() << Q_FUNC_INFO;

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    // qDebug() << "Resolved services...";
    // qDebug().noquote() << xml.value();

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    // qDebug() << nodes.count() << "nodes";

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
            } else if (uuid == PebbleService::UUID_SERVICE_PEBBLE && !service(PebbleService::UUID_SERVICE_PEBBLE)) {
                addService(PebbleService::UUID_SERVICE_PEBBLE, new PebbleService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void PebbleDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &PebbleDevice::informationChanged, Qt::UniqueConnection);
    }
}

void PebbleDevice::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;
    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        info->refreshInformation();
    }
}

QString PebbleDevice::information(Info i) const
{
    qDebug() << Q_FUNC_INFO << i;

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (!info) {
        qWarning() << "Device info service doesn't exists";
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
