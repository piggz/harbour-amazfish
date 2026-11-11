#include "garmindevice.h"
#include "services/garmin/communicator_v2.h"
#include "hrmservice.h"
#include "deviceinfoservice.h"
#include "amazfishconfig.h"


#include <QDomDocument>
#include <QStringList>

const char* UUID_SERVICE_GARMIN_GFDI_V0 = "9b012401-bc30-ce9a-e111-0f67e491abde";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V0_SEND = "df334c80-e6a7-d082-274a-78fc66f85e16";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V0_RECEIVE = "4acbcd28-7425-868e-f447-915c8f00d0cb";

const char* UUID_SERVICE_GARMIN_GFDI_V1 = "6a4e2401-667b-11e3-949a-0800200c9a66";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V1_SEND = "6a4e4c80-667b-11e3-949a-0800200c9a66";
const char* UUID_CHARACTERISTIC_GARMIN_GFDI_V1_RECEIVE = "6a4ecd28-667b-11e3-949a-0800200c9a66";

const char* UUID_SERVICE_GARMIN_ML_GFDI = "6a4e2800-667b-11e3-949a-0800200c9a66";

GarminDevice::GarminDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &GarminDevice::onPropertiesChanged, Qt::UniqueConnection);
    m_pairing= false;
}

Amazfish::Features GarminDevice::supportedFeatures() const
{

    //HRM and steps should be supported on all devices

    return  Amazfish::Feature::FEATURE_NONE
        | Amazfish::Feature::FEATURE_HRM
        | Amazfish::Feature::FEATURE_STEPS
        | Amazfish::Feature::FEATURE_ALERT
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
    qDebug() << Q_FUNC_INFO << "notification from " << notification.appName;
    if (mNotificationHandler)
    {
        NotificationSpec note;
        note.body = notification.body;
        note.sourceName = notification.appName;
        note.title = notification.summary;
        // Configure notification type based on Appname (small list so far, needs to be extended);
        if (notification.appName=="Whisperfish")
            note.notificationType=NotificationType::GenericSocial;
        else if (notification.appName=="Signal")
            note.notificationType=NotificationType::GenericSocial;
        // It seems for emails we don't see the app name but the sender address. Bug or feature?
        else if (notification.appName.contains('@'))
            note.notificationType=NotificationType::GenericEmail;
        else if (notification.appName=="Calendar")
            note.notificationType=NotificationType::GenericCalendar;
        else note.notificationType = NotificationType::Generic;
        note.id=notification.id;
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

void GarminDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list;

    if (interface == "org.bluez.Device1") {
        // Captured before initialise(): on a first connect that call creates the communicator and
        // its constructor talks to the watch. Only a reconnect has nothing created and needs a re-run.
        CommunicatorV2* svc = mCommunicator.data();
        if (deviceProperty("ServicesResolved").toBool() ) {
            initialise();
            if (svc) {
                qDebug() << Q_FUNC_INFO << "Garmin: Communicator already exists, re-initialising";
                svc->onConnectionStateChange(true);
                // drv_createService() set this state on the first connect and is not reached again.
                setConnectionState("authenticated");
            }
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();
            if (!value) {
                setConnectionState("disconnected");
                if (svc)
                {
                    qDebug() << Q_FUNC_INFO << "Garmin: Communicator already exists, sending disconnect";
                    svc->onConnectionStateChange(false);
                }
            } else {
                setConnectionState("connected");
            }
        }
    }
}
void GarminDevice::onAnswerCallEvent() {
    emit deviceEvent(AbstractDevice::EVENT_ANSWER_CALL);
}

void GarminDevice::onRejectCallEvent(){
    emit deviceEvent(AbstractDevice::EVENT_DECLINE_CALL);
}

QBLEService *GarminDevice::drv_createService(const QString &uuid, const QString &path)
{
    qDebug() << Q_FUNC_INFO << "Creating Communicator service for Garmin";
    // Only the preferred GFDI generation gets a CommunicatorV2.
    if (uuid != mPreferredService) {
        return nullptr;
    }

    QSharedPointer<CommunicatorV2> com = QSharedPointer<CommunicatorV2>::create(path, this);
    if (com) {
        mCommunicator = com;
        connect(com.data(), &CommunicatorV2::informationChanged, this, &GarminDevice::informationChanged, Qt::UniqueConnection);
        // add notification handler
        qDebug() << Q_FUNC_INFO << "Garmin: Adding notification handler";
        if (!mNotificationHandler)
            mNotificationHandler = QSharedPointer<GarminNotificationHandler>::create(mCommunicator);
        connect(com.data(), &CommunicatorV2::NotificationDataRequested, mNotificationHandler.data(), &GarminNotificationHandler::onNotificationDataRequested);
        connect(com.data(), &CommunicatorV2::NotificationPerformAction, mNotificationHandler.data(), &GarminNotificationHandler::onNotificationPerformAction);
        connect(mNotificationHandler.data(), &GarminNotificationHandler::acceptIncomingCall, this, &GarminDevice::onAnswerCallEvent);
        connect(mNotificationHandler.data(), &GarminNotificationHandler::rejectIncomingCall, this, &GarminDevice::onRejectCallEvent);
        setConnectionState("authenticated");
        return com.data();
    }
    // if we are here, no Garmin device was detected or serice already created
    return nullptr;
}

void GarminDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    // A watch can advertise more than one GFDI generation. Use the newest one available.
    // The last case is unconditional: if BlueZ reports no UUIDs, the V0 link is still the
    // safest guess.
    const QStringList advertised = deviceProperty("UUIDs").toStringList();
    if (advertised.contains(UUID_SERVICE_GARMIN_ML_GFDI)) {
        mPreferredService = UUID_SERVICE_GARMIN_ML_GFDI;
    } else if (advertised.contains(UUID_SERVICE_GARMIN_GFDI_V1)) {
        mPreferredService = UUID_SERVICE_GARMIN_GFDI_V1;
    } else {
        mPreferredService = UUID_SERVICE_GARMIN_GFDI_V0;
    }
    parseServices();
}


void GarminDevice::refreshInformation()
{

}

QString GarminDevice::information(Amazfish::Info i) const
{

    if (!mCommunicator.data()) {
        qDebug() << Q_FUNC_INFO << "No communicator found!";
        return QString();
    }

    struct deviceInfo info=mCommunicator.data()->deviceInfo();
    switch(i) {
    case Amazfish::Info::INFO_SWVER:
        return info.softwareRevision;
        break;
    case Amazfish::Info::INFO_SERIAL:
        return info.serialNumber;
        break;
    case Amazfish::Info::INFO_BATTERY:
        return QString::number(mCommunicator.data()->batteryLevel());
        break;
    case Amazfish::Info::INFO_MODEL:
        return info.deviceModel;
        break;
    case Amazfish::Info::INFO_MANUFACTURER:
        return info.deviceManufacturer;
    case Amazfish::Info::INFO_STEPS:
        qDebug() << Q_FUNC_INFO << "Steps: " << mCommunicator.data()->steps();
        return QString::number(mCommunicator.data()->steps());
        break;
    case Amazfish::Info::INFO_HEARTRATE:
        qDebug() << Q_FUNC_INFO << "Heart rate: " << mCommunicator.data()->heartRate();
        return QString::number(mCommunicator.data()->heartRate());
        break;
    default:
        return QString("");
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
