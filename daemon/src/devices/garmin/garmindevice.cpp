#include "garmindevice.h"
#include "services/garmin/communicator_v2.h"
#include "hrmservice.h"
#include "deviceinfoservice.h"
#include "amazfishconfig.h"


#include <QDomDocument>



GarminDevice::GarminDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &GarminDevice::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features GarminDevice::supportedFeatures() const
{

    return  Amazfish::Feature::FEATURE_NONE
         | Amazfish::Feature::FEATURE_HRM
        // | Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS
        // | Amazfish::Feature::FEATURE_ALARMS
        //| Amazfish::Feature::FEATURE_ALERT
        // | Amazfish::Feature::FEATURE_EVENT_REMINDER
        // | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        // | Amazfish::Feature::FEATURE_BUTTON_ACTION
        // | Amazfish::Feature::FEATURE_SCREENSHOT
        // | Amazfish::Feature::FEATURE_FILE_INSTALL
        ;
}


Amazfish::DataTypes GarminDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_SPO2
            | Amazfish::DataType::TYPE_HRV
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

}

void GarminDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
}

void GarminDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
}


void GarminDevice::pair()
{
    qDebug() << Q_FUNC_INFO << "Pairing with Garmin Epix";

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
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
    // Garmin is using a single service for all functions (Mlr), so we probably don't need full parsing.
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
            if (uuid == CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI && !service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI)) {
                qDebug() << "Added Garmin ML GDFI Service";
                CommunicatorV2 *com = new CommunicatorV2(path, this);
                if (com)
                {
                    connect(com, &CommunicatorV2::informationChanged, this, &GarminDevice::informationChanged, Qt::UniqueConnection);
                    addService(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI, com);

                }
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

}


void GarminDevice::refreshInformation()
{

}


QString GarminDevice::information(Amazfish::Info i) const
{

    CommunicatorV2 *com = qobject_cast<CommunicatorV2*>(service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI));
    if (!com) {
        qDebug() << Q_FUNC_INFO << "No communicator found!";
        return QString();
    }


    struct deviceInfo info=com->deviceInfo();
    switch(i) {
    case Amazfish::Info::INFO_SWVER:
        return info.softwareRevision;
        break;
    case Amazfish::Info::INFO_SERIAL:
        return info.serialNumber;
        break;
    case Amazfish::Info::INFO_BATTERY:
        return QString::number(com->batteryLevel());
        break;
    case Amazfish::Info::INFO_MODEL:
        return info.deviceModel;
        break;
    case Amazfish::Info::INFO_MANUFACTURER:
        return info.deviceManufacturer;

    case Amazfish::Info::INFO_STEPS:
        qDebug() << Q_FUNC_INFO << "Steps: " << com->steps();
        return QString::number(com->steps());
        break;
    case Amazfish::Info::INFO_HEARTRATE:
        qDebug() << Q_FUNC_INFO << "Heart rate: " << com->heartRate();
        return QString::number(com->heartRate());
        break;
    }
    return QString();
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
