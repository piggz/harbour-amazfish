#include "zepposdevice.h"
#include "batteryservice.h"

#include "zeppos/zepposauthservice.h"
#include "zeppos/zepposservicesservice.h"
#include "zeppos/zepposnotificationservice.h"
#include "zeppos/zepposstepsservice.h"
#include "zeppos/zepposheartrateservice.h"

#include <QtXml/QtXml>
#include <QDebug>

ZeppOSDevice::ZeppOSDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;

    m_ActivitySampleSize = 8;

    connect(this, &QBLEDevice::propertiesChanged, this, &ZeppOSDevice::onPropertiesChanged);

    //Create all possile services

    m_servicesService = new ZeppOsServicesService(this);
    m_serviceMap[m_servicesService->endpoint()] = m_servicesService;

    m_authService = new ZeppOsAuthService(this);
    m_serviceMap[m_authService->endpoint()] = m_authService;

    m_notificationService = new ZeppOsNotificationService(this);
    m_serviceMap[m_notificationService->endpoint()] = m_notificationService;

    m_stepsService = new ZeppOsStepsService(this);
    m_serviceMap[m_stepsService->endpoint()] = m_stepsService;

    m_heartRateService = new ZeppOsHeartRateService(this);
    m_serviceMap[m_heartRateService->endpoint()] = m_heartRateService;
}

QString ZeppOSDevice::deviceType() const
{
    return "amazfitbalance";
}

int ZeppOSDevice::supportedFeatures() const
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

AbstractFirmwareInfo *ZeppOSDevice::firmwareInfo(const QByteArray &bytes)
{
    return nullptr;
}

void ZeppOSDevice::sendAlert(const Amazfish::WatchNotification &notification)
{
    qDebug() << Q_FUNC_INFO;
    m_notificationService->sendAlert(notification);
}

void ZeppOSDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO;
    m_notificationService->incomingCall(caller);
}

void ZeppOSDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
}

void ZeppOSDevice::requestManualHeartrate() const
{
    m_heartRateService->enableRealtimeHeartRateMeasurement(true, true);
}

void ZeppOSDevice::writeToChunked2021(short endpoint, QByteArray data, bool encryptIgnored)
{
    m_encoder->write(endpoint, data, true, encryptIgnored);
}

AbstractZeppOsService *ZeppOSDevice::zosService(short endpoint) const
{
    if (m_serviceMap.contains(endpoint)) {
        qDebug() << "Found matching service";
        return m_serviceMap[endpoint];
    }
    return nullptr;
}

void ZeppOSDevice::addSupportedService(short endpoint, bool encryted)
{
    qDebug() << Q_FUNC_INFO << endpoint << encryted;
    m_supportedServices.insert(endpoint);
}

void ZeppOSDevice::handle2021Payload(short type, const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO << type;

    if (data.isEmpty()) {
        qDebug() << "Empty payload for " << type;
        return;
    }

    AbstractZeppOsService *service = zosService(type);
    if (service) {
        qDebug() << "Found service " << service->name();
        service->handlePayload(data);
        return;
    }
}

void ZeppOSDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;


    if (ready) {
        m_servicesService->requestServices();
    } else {
        setConnectionState("authfailed");
    }
}

void ZeppOSDevice::ready()
{
    setConnectionState("authenticated");

    m_stepsService->enableRealtimeSteps(true);
}

void ZeppOSDevice::setEncryptionParameters(int encryptedSequenceNumber, QByteArray sharedSessionKey)
{
    m_encoder->setEncryptionParameters(encryptedSequenceNumber, sharedSessionKey);
    m_decoder->setEncryptionParameters(sharedSessionKey);
}

void ZeppOSDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << Q_FUNC_INFO << interface << map << list << m_connectionState;

    if (interface == "org.bluez.Device1") {
        m_reconnectTimer->start();
        if (map.contains("Paired")) {
            bool value = map["Paired"].toBool();

            if (value) {
                setConnectionState("paired");
            }
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (!value) {
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
        if (deviceProperty("ServicesResolved").toBool() ) {
            int elapsed = init_dt.secsTo(QDateTime::currentDateTime());
            qDebug() << "initialise() elapsed: " << elapsed << "starting: " << (elapsed >60);
            if (elapsed > 60) {
                init_dt = QDateTime::currentDateTime();
                initialise();
            }
        }
    }
}

void ZeppOSDevice::parseServices()
{
    qDebug() << Q_FUNC_INFO;

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
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this, true));
            } else if (uuid == MiBandService::UUID_SERVICE_MIBAND && !service(MiBandService::UUID_SERVICE_MIBAND)) {
                addService(MiBandService::UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == MiBand2Service::UUID_SERVICE_MIBAND2 && !service(MiBand2Service::UUID_SERVICE_MIBAND2)) {
                addService(MiBand2Service::UUID_SERVICE_MIBAND2, new MiBand2Service(path, 0x00, 0x80, true, this));
            } else if (uuid == BipFirmwareService::UUID_SERVICE_FIRMWARE && !service(BipFirmwareService::UUID_SERVICE_FIRMWARE)) {
                addService(BipFirmwareService::UUID_SERVICE_FIRMWARE, new BipFirmwareService(path, this));
            } else if (uuid == BatteryService::UUID_SERVICE_BATTERY && !service(BatteryService::UUID_SERVICE_BATTERY)) {
                addService(BatteryService::UUID_SERVICE_BATTERY, new BatteryService(path, this));
            } else if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void ZeppOSDevice::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex();

    if (characteristic == HRMService::UUID_CHARACTERISTIC_HRM_MEASUREMENT) {
        m_heartRateService->handleHeartRate(value);
    }
}

void ZeppOSDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER);

        m_encoder = new Huami2021ChunkedEncoder(mi->characteristic(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE), true);
        m_decoder = new Huami2021ChunkedDecoder(this, true);

        mi->setHuami2021ChunkedDecoder(m_decoder);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &ZeppOSDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &ZeppOSDevice::serviceEvent, Qt::UniqueConnection);


        m_authService->startAuthentication();
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(MiBand2Service::UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got MiBand2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &HuamiDevice::authenticated, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(BipFirmwareService::UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(fw, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(DeviceInfoService::UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(HRMService::UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(hrm, &QBLEService::characteristicChanged, this, &ZeppOSDevice::characteristicChanged, Qt::UniqueConnection);
    }
}
