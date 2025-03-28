#include "zepposdevice.h"
#include "amazfishconfig.h"

#include <QtXml/QtXml>
#include <QDebug>

ZeppOSDevice::ZeppOSDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
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

}

void ZeppOSDevice::incomingCall(const QString &caller)
{

}

void ZeppOSDevice::incomingCallEnded()
{

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

            if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
}

void ZeppOSDevice::initialise()
{
    qDebug() << Q_FUNC_INFO;
    parseServices();

}
