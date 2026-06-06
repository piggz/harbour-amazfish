#include "garmindevice.h"
#include "services/garmin/communicator_v2.h"
#include "hrmservice.h"
#include "deviceinfoservice.h"
#include "amazfishconfig.h"


#include <QDomDocument>

const char* UUID_SERVICE_GARMIN_GFDI_V0 = "9b012401-bc30-ce9a-e111-0f67e491abde";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V0_SEND = "df334c80-e6a7-d082-274a-78fc66f85e16";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V0_RECEIVE = "4acbcd28-7425-868e-f447-915c8f00d0cb";

const char* UUID_SERVICE_GARMIN_GFDI_V1 = "6a4e2401-667b-11e3-949a-0800200c9a66";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V1_SEND = "6a4e4c80-667b-11e3-949a-0800200c9a66";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V1_RECEIVE = "6a4ecd28-667B-11e3-949a-0800200c9a66";

GarminDevice::GarminDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &GarminDevice::onPropertiesChanged, Qt::UniqueConnection);
}

Amazfish::Features GarminDevice::supportedFeatures() const
{

    //HRM and steps should be supported on all devices

    return  Amazfish::Feature::FEATURE_NONE
        | Amazfish::Feature::FEATURE_HRM
        // | Amazfish::Feature::FEATURE_ACTIVITY
        | Amazfish::Feature::FEATURE_STEPS
        // | Amazfish::Feature::FEATURE_ALARMS
        | Amazfish::Feature::FEATURE_ALERT
        // | Amazfish::Feature::FEATURE_EVENT_REMINDER
        // | Amazfish::Feature::FEATURE_MUSIC_CONTROL
        // | Amazfish::Feature::FEATURE_BUTTON_ACTION
        // | Amazfish::Feature::FEATURE_SCREENSHOT
        // | Amazfish::Feature::FEATURE_FILE_INSTALL
        // Amazfish::Feature::FEATURE_SPO2
        ;
}


Amazfish::DataTypes GarminDevice::supportedDataTypes() const
{
    return Amazfish::DataType::TYPE_SPO2
            | Amazfish::DataType::TYPE_HRV
            | Amazfish::DataType::TYPE_HEART_RATE
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
    if (mNotificationHandler)
    {
        NotificationSpec note;
        note.body = notification.body;
        note.sourceName = notification.appName;
        note.title = notification.summary;
        note.id=0;
        mNotificationHandler->onNotification(note);
    }
    else qDebug() << Q_FUNC_INFO << "Garmin: No notification handler!";
}

void GarminDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO << caller;
    if (mNotificationHandler)
    {
        CallSpec call;
        call.name = caller;
        call.command = CallCommand::Incoming;
        mNotificationHandler->onSetCallState(call);
    }
    else qDebug() << Q_FUNC_INFO << "Garmin: No notification handler!";
}

void GarminDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
    if (mNotificationHandler)
    {
        CallSpec call;
        call.command = CallCommand::End;
        mNotificationHandler->onSetCallState(call);
    }
    else qDebug() << Q_FUNC_INFO << "Garmin: No notification handler!";
}


void GarminDevice::pair()
{
    qDebug() << Q_FUNC_INFO << "Pairing with Garmin " << devicePath();

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;


    setConnectionState("pairing");
    //emit connectionStateChanged();
    QBLEDevice::pair();
    //initialise();
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
            CommunicatorV2 *svc = qobject_cast<CommunicatorV2 *>(service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI));
            if (!value) {
                setConnectionState("disconnected");
                if (svc)
                {
                    qDebug() << Q_FUNC_INFO << "Garmin: Communicator already exists, sending disconnect";
                    svc->onConnectionStateChange(false);
                }
            } else {
                setConnectionState("connected");
                if (svc)
                {
                    qDebug() << Q_FUNC_INFO << "Garmin: Communicator already exists, sending connect";
                    svc->onConnectionStateChange(true);
                }
            }
        }
    }
}


void GarminDevice::parseServices()
{
    // Garmin is using a single service for all functions (Mlr), so we probably don't need full parsing.
    qDebug() << Q_FUNC_INFO << "Parsing Services for Garmin";
    CommunicatorV2* com = qobject_cast<CommunicatorV2*> (service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI));
    if (com)
    //if (service(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI))
    {
        qDebug() << Q_FUNC_INFO << "Garmin: Communicator already exists, no parsing required.";
        //re-initialise device
        com->initializeDevice();
    }
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
            if (uuid == CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI) {
                qDebug() << "Added Garmin ML GDFI Service";
                QSharedPointer<CommunicatorV2> com = QSharedPointer<CommunicatorV2>::create(path, this);
                if (com)
                {
                    connect(com.data(), &CommunicatorV2::informationChanged, this, &GarminDevice::informationChanged, Qt::UniqueConnection);
                    addService(CommunicatorV2::UUID_SERVICE_GARMIN_ML_GFDI, com.data());

                    // add notification handler
                    qDebug() << Q_FUNC_INFO << "Garmin: Adding notification handler";
                    mNotificationHandler = QSharedPointer<GarminNotificationHandler>::create(com);
                    //connect(this,&GarminDevice::sendAlertToDevice,mNotificationHandler.data(),&GarminNotificationHandler::onNotification);
                    setConnectionState("connected");
                    return;
                }
            }
            if ((uuid == UUID_SERVICE_GARMIN_GFDI_V0) ||(uuid == UUID_SERVICE_GARMIN_GFDI_V1))
            {
                emit message("Garmin V0/V1 protocol not implemented");
                qDebug() << Q_FUNC_INFO << "Garmin: Protocol version 0/1 sound, not supported yet";
                return;
            }
        }
    }
    // if we are here, no Garmin device was detected

    emit message("No Garmin device detected");
    qDebug() << Q_FUNC_INFO << "Garmin: No supported device detected";

}

void GarminDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    //setConnectionState("connected");
    parseServices();

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
