#include "zepposdevice.h"
#include "batteryservice.h"
#include "huamiinitoperation2021.h"

#include "zeppos/zepposnotificationservice.h"

#include <QtXml/QtXml>
#include <QDebug>

ZeppOSDevice::ZeppOSDevice(const QString &pairedName, QObject *parent) : HuamiDevice(pairedName, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEDevice::propertiesChanged, this, &ZeppOSDevice::onPropertiesChanged);
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

void ZeppOSDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << Q_FUNC_INFO;
    notificationService->sendAlert(sender, subject, message);
}

void ZeppOSDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO;
}

void ZeppOSDevice::incomingCallEnded()
{
    qDebug() << Q_FUNC_INFO;
}

void ZeppOSDevice::writeToChunked2021(short endpoint, QByteArray data, bool encryptIgnored)
{
    m_encoder->write(endpoint, data, true, encryptIgnored);
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
                addService(HRMService::UUID_SERVICE_HRM, new HRMService(path, this));
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

void ZeppOSDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(MiBandService::UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER);

        m_encoder = new Huami2021ChunkedEncoder(mi->characteristic(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE), true);
        m_decoder = new Huami2021ChunkedDecoder(true);

        connect(mi, &MiBandService::message, this, &HuamiDevice::message, Qt::UniqueConnection);
        connect(mi, &AbstractOperationService::operationRunningChanged, this, &AbstractDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &ZeppOSDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &HuamiDevice::informationChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::serviceEvent, this, &ZeppOSDevice::serviceEvent, Qt::UniqueConnection);

        HuamiInitOperation2021 *init = new HuamiInitOperation2021(true, 0x00, 0x80, this);
        mi->registerOperation(init);

        notificationService = new ZeppOsNotificationService(this, true);
        init->start(mi);
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
    }
}
