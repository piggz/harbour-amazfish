#include "gtsdevice.h"
#include "gtsfirmwareinfo.h"
#include <QtXml/QtXml>

GtsDevice::GtsDevice(const QString &pairedName, QObject *parent) : BipDevice(pairedName, parent)
{
    qDebug() << "Creating GTS Device";
}

QString GtsDevice::deviceType()
{
    return "amazfitgts";
}

void GtsDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
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

void GtsDevice::initialise()
{
    setConnectionState("connected");
    parseServices();

    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(mi, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
        connect(mi, &MiBandService::buttonPressed, this, &GtsDevice::handleButtonPressed, Qt::UniqueConnection);
        connect(mi, &MiBandService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }

    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(UUID_SERVICE_MIBAND2));
    if (mi2) {
        qDebug() << "Got mi2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &BipDevice::authenticated, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(m_needsAuth);
    }

    BipFirmwareService *fw = qobject_cast<BipFirmwareService*>(service(UUID_SERVICE_FIRMWARE));
    if (fw) {
        connect(fw, &BipFirmwareService::message, this, &BipDevice::message, Qt::UniqueConnection);
        connect(fw, &BipFirmwareService::downloadProgress, this, &BipDevice::downloadProgress, Qt::UniqueConnection);
        connect(mi2, &QBLEService::operationRunningChanged, this, &QBLEDevice::operationRunningChanged, Qt::UniqueConnection);
    }

    DeviceInfoService *info = qobject_cast<DeviceInfoService*>(service(UUID_SERVICE_DEVICEINFO));
    if (info) {
        connect(info, &DeviceInfoService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }

    HRMService *hrm = qobject_cast<HRMService*>(service(UUID_SERVICE_HRM));
    if (hrm) {
        connect(hrm, &HRMService::informationChanged, this, &BipDevice::informationChanged, Qt::UniqueConnection);
    }
}


void GtsDevice::parseServices()
{
    qDebug() << "GtsDevice::parseServices";

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

            if (uuid == UUID_SERVICE_ALERT_NOTIFICATION && !service(UUID_SERVICE_ALERT_NOTIFICATION)) {
                addService(UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == UUID_SERVICE_DEVICEINFO  && !service(UUID_SERVICE_DEVICEINFO)) {
                addService(UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == UUID_SERVICE_HRM && !service(UUID_SERVICE_HRM)) {
                addService(UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND && !service(UUID_SERVICE_MIBAND)) {
                addService(UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND2 && !service(UUID_SERVICE_MIBAND2)) {
                addService(UUID_SERVICE_MIBAND2, new MiBand2Service(path, 0x00, 0x80, true, this));
            } else if (uuid == UUID_SERVICE_FIRMWARE && !service(UUID_SERVICE_FIRMWARE)) {
                addService(UUID_SERVICE_FIRMWARE, new BipFirmwareService(path, this));
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
