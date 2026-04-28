#include "garmindevice.h"

#include "hrmservice.h"
#include "deviceinfoservice.h"
#include <QDomDocument>



GarminDevice::GarminDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &GarminDevice::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features GarminDevice::supportedFeatures() const
{

    return  Amazfish::Feature::FEATURE_HRM
        // | Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS
        // | Amazfish::Feature::FEATURE_ALARMS
        | Amazfish::Feature::FEATURE_ALERT
        // | Amazfish::Feature::FEATURE_EVENT_REMINDER
        // | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        // | Amazfish::Feature::FEATURE_BUTTON_ACTION
        // | Amazfish::Feature::FEATURE_SCREENSHOT
        // | Amazfish::Feature::FEATURE_FILE_INSTALL
        ;
}

QString GarminDevice::deviceType() const
{
    return "asteroidos";
}

AbstractFirmwareInfo *GarminDevice::firmwareInfo(const QByteArray &bytes, const QString &path)
{
    qDebug() << Q_FUNC_INFO;
    return nullptr;
}

void GarminDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    qDebug() << Q_FUNC_INFO;
/*
    AsteroidNotificationService *n = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (n) {
        n->sendAlert(notification);
    }
*/
}

void GarminDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
/*
    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
        notification->incomingCall(caller);
    }
*/
}

void GarminDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
/*
    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
        notification->incomingCallEnded();
    }
*/
}


void GarminDevice::pair()
{
    qDebug() << Q_FUNC_INFO << "Pairing with Garmin Epix";

    m_needsAuth = false;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
    initialise();
}


void GarminDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
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


void GarminDevice::parseServices()
{
    qDebug() << Q_FUNC_INFO << "Parsing Services for Garmin Epix";

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

            if (uuid == HRMService::UUID_SERVICE_HRM && !service(HRMService::UUID_SERVICE_HRM)) {
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
            }
/*
            if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
                addService(BatteryService::UUID_SERVICE_BATTERY, new BatteryService(path, this));
            } else if (uuid == AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME && !service(AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME)) {
                addService(AsteroidTimeService::UUID_SERVICE_ASTEROID_TIME, new AsteroidTimeService(path, this));
            } else if (uuid == AsteroidWeatherService::UUID_SERVICE_WEATHER && !service(AsteroidWeatherService::UUID_SERVICE_WEATHER)) {
                addService(AsteroidWeatherService::UUID_SERVICE_WEATHER, new AsteroidWeatherService(path, this));
            } else if (uuid == AsteroidNotificationService::UUID_SERVICE_NOTIFICATION && !service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION)) {
                addService(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION, new AsteroidNotificationService(path, this));
            } else if (uuid == AsteroidMediaService::UUID_SERVICE_MEDIA  && !service(AsteroidMediaService::UUID_SERVICE_MEDIA  )) {
                addService(AsteroidMediaService::UUID_SERVICE_MEDIA  , new AsteroidMediaService(path, this));
            } else if (uuid == AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT  && !service(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT  )) {
                addService(AsteroidScreenshotService::UUID_SERVICE_SCREENSHOT, new AsteroidScreenshotService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
 */
        }
    }
    setConnectionState("authenticated");
}

void GarminDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();
/*
    AsteroidNotificationService *notification = qobject_cast<AsteroidNotificationService*>(service(AsteroidNotificationService::UUID_SERVICE_NOTIFICATION));
    if (notification) {
//        notification->enableNotification(AsteroidNotificationService::UUID_CHARACTERISTIC_ALERT_NOTIFICATION_EVENT);
//        connect(alert, &AlertNotificationService::serviceEvent, this, &PinetimeJFDevice::serviceEvent, Qt::UniqueConnection);
    }
*/

    /*
    BatteryService *battery = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (battery) {
        connect(battery, &BatteryService::informationChanged, this, &GarminDevice::informationChanged, Qt::UniqueConnection);
    }
    */
}


void GarminDevice::refreshInformation()
{
/*
    BatteryService *bat = qobject_cast<BatteryService*>(service(BatteryService::UUID_SERVICE_BATTERY));
    if (bat) {
        bat->refreshInformation();
    }
*/
}


Amazfish::DataTypes GarminDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_NONE;
}
