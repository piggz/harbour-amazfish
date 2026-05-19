#include "garmindevice.h"
#include "services/garmin/communicator_v2.h"
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

    return  Amazfish::Feature::FEATURE_NONE
        // | Amazfish::Feature::FEATURE_HRM
        // | Amazfish::Feature::FEATURE_ACTIVITY
        //| Amazfish::Feature::FEATURE_STEPS
        // | Amazfish::Feature::FEATURE_ALARMS
        //| Amazfish::Feature::FEATURE_ALERT
        // | Amazfish::Feature::FEATURE_EVENT_REMINDER
        // | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        // | Amazfish::Feature::FEATURE_BUTTON_ACTION
        // | Amazfish::Feature::FEATURE_SCREENSHOT
        // | Amazfish::Feature::FEATURE_FILE_INSTALL
        ;
}

QString GarminDevice::deviceType() const
{
    return "garmin";
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

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
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
    // Garmin is using a single service for all functions (Mlr), so we don't need that.
    qDebug() << Q_FUNC_INFO << "Parsing Services for Garmin Epix";

    QDBusInterface adapterIntro("org.bluez", devicePath(), "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");


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
                qDebug() << "Added Garmin HRM Support";
            }
            else if (uuid == CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI && !service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI)) {
                qDebug() << "Added Garmin ML GDFI Service";
                addService(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI, new CommunicatorV2(path, this));
            }
            else{
                 //addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void GarminDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    setConnectionState("connected");
    parseServices();

    setConnectionState("authenticated");
    //mSteps = getStepsFromDb();
    //emit informationChanged(Amazfish::Info::INFO_STEPS, QString::number(mSteps));

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


void GarminDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}
