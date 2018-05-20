#include "bipdevice.h"
#include <QtXml/QtXml>

const char* BipDevice::UUID_SERVICE_ALERT_NOTIFICATION = AlertNotificationService::UUID_SERVICE_ALERT_NOTIFICATION;
const char* BipDevice::UUID_SERVICE_DEVICEINFO = DeviceInfoService::UUID_SERVICE_DEVICEINFO;
const char* BipDevice::UUID_SERVICE_HRM = HRMService::UUID_SERVICE_HRM;
const char* BipDevice::UUID_SERVICE_MIBAND = MiBandService::UUID_SERVICE_MIBAND;
const char* BipDevice::UUID_SERVICE_MIBAND2 = MiBand2Service::UUID_SERVICE_MIBAND2;

BipDevice::BipDevice()
{
    setConnectionState("disconnected");
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(60000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &BipDevice::reconnectionTimer);
}

void BipDevice::pair()
{
    qDebug() << "BipDevice::pair";

    m_needsAuth = true;
    m_pairing = true;
    m_autoreconnect = true;
    disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::pair();
}

void BipDevice::connectToDevice()
{
    qDebug() << "BipDevice::connectToDevice";

    m_pairing = false;
    m_autoreconnect = true;
    QBLEDevice::disconnectFromDevice();
    setConnectionState("connecting");
    QBLEDevice::connectToDevice();
    m_reconnectTimer->start(); //Start timer to attempt to reconnect every 60 seconds
}

void BipDevice::disconnectFromDevice()
{
    qDebug() << "BipDevice::disconnectFromDevice";

    m_autoreconnect = false;
    setConnectionState("disconnected");

    QBLEDevice::disconnectFromDevice();
}

void BipDevice::parseServices()
{
    qDebug() << "BipDevice::parseServices";

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

            if (uuid == UUID_SERVICE_ALERT_NOTIFICATION) {
                addService(UUID_SERVICE_ALERT_NOTIFICATION, new AlertNotificationService(path, this));
            } else if (uuid == UUID_SERVICE_DEVICEINFO) {
                addService(UUID_SERVICE_DEVICEINFO, new DeviceInfoService(path, this));
            } else if (uuid == UUID_SERVICE_HRM) {
                addService(UUID_SERVICE_HRM, new HRMService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND) {
                addService(UUID_SERVICE_MIBAND, new MiBandService(path, this));
            } else if (uuid == UUID_SERVICE_MIBAND2) {
                addService(UUID_SERVICE_MIBAND2, new MiBand2Service(path, this));
            } else {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void BipDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
{
    qDebug() << "BipDevice::onPropertiesChanged:" << interface << map << list;

    if (interface == "org.bluez.Device1") {
        if (map.contains("Paired")) {
            bool value = map["Paired"].toBool();

            if (value) {
                qDebug() << "Paired!";
            }
        }
        if (map.contains("Connected")) {
            bool value = map["Connected"].toBool();

            if (value) {
                qDebug() << "Connected!";
                //Check if services are resolved
                bool resolved = deviceProperty("ServicesResolved").toBool();
                qDebug() << "Resolved:" << resolved;

                if (resolved) {
                    initialise();
                }
            }
        }
        if (map.contains("ServicesResolved")) {
            bool value = map["ServicesResolved"].toBool();

            if (value && !m_pairing) {
                qDebug() << "ServicesResolved!";
                initialise();
            }
        }
    }

}

void BipDevice::authenticated(bool ready)
{
    qDebug() << "BipInterface::authenticated:" << ready;

    if (ready) {
        m_needsAuth = false;
        setConnectionState("authenticated");

        AlertNotificationService *alert = qobject_cast<AlertNotificationService*>(service(UUID_SERVICE_ALERT_NOTIFICATION));

        if (alert) {
            alert->sendAlert(tr("Amazfish"), tr("Connected"), tr("Phone and watch are connected"), true);
        }

        MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
        if (mi){
            mi->setCurrentTime();
            mi->setLanguage();
            mi->setDateDisplay();
            mi->setTimeFormat();
            mi->setUserInfo();
            mi->setDisplayCaller();
            mi->setRotateWristToSwitchInfo(true);
            mi->setGoalNotification(true);
        }
    } else {
        setConnectionState("authfailed");
    }
}

void BipDevice::setConnectionState(const QString &state)
{
    if (state != m_connectionState) {
        m_connectionState = state;
        emit connectionStateChanged();
    }
}

QString BipDevice::connectionState() const
{
    return m_connectionState;
}

void BipDevice::initialise()
{
    setConnectionState("connected");
    parseServices();
    MiBandService *mi = qobject_cast<MiBandService*>(service(UUID_SERVICE_MIBAND));
    MiBand2Service *mi2 = qobject_cast<MiBand2Service*>(service(UUID_SERVICE_MIBAND2));

    if (mi) {
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_CONFIGURATION);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_BATTERY_INFO);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_DEVICE_EVENT);
        mi->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_REALTIME_STEPS);

        connect(mi, &MiBandService::message, this, &BipDevice::message);

    }
    if (mi2) {
        qDebug() << "Got mi2 service";
        connect(mi2, &MiBand2Service::authenticated, this, &BipDevice::authenticated);

        mi2->enableNotification(MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH);
        mi2->initialise(m_needsAuth);
    } else {
        qDebug() << "NOT got mi2 service!";
    }
}

void BipDevice::reconnectionTimer()
{
    qDebug() << "BipDevice::reconnectionTimer";
    if ((!deviceProperty("Connected").toBool() && m_autoreconnect) || connectionState() == "authfailed") {
        qDebug() << "Lost connection";
        QBLEDevice::connectToDevice();
    }
}
