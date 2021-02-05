#include "banglejsdevice.h"

#include <QtXml/QtXml>

BangleJSDevice::BangleJSDevice(const QString &pairedName, QObject *parent) : AbstractDevice(pairedName, parent)
{
    qDebug() << "PinetimeJFDevice:: " << pairedName;
    connect(this, &QBLEDevice::propertiesChanged, this, &BangleJSDevice::onPropertiesChanged, Qt::UniqueConnection);
}

QString BangleJSDevice::pair()
{
    qDebug() << Q_FUNC_INFO;

    m_needsAuth = false;
    m_pairing = true;
    m_autoreconnect = true;
    //disconnectFromDevice();
    setConnectionState("pairing");
    emit connectionStateChanged();

    QBLEDevice::connectToDevice();
    return "pairing";
}

int BangleJSDevice::supportedFeatures()
{
    return FEATURE_HRM |
            FEATURE_ALERT;
}

QString BangleJSDevice::deviceType()
{
    return "pinetimejf";
}

bool BangleJSDevice::operationRunning()
{
    return QBLEDevice::operationRunning();
}

void BangleJSDevice::abortOperations()
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::sendAlert(const QString &sender, const QString &subject, const QString &message)
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::incomingCall(const QString &caller)
{
    qDebug() << Q_FUNC_INFO;

    Q_UNUSED(caller);
}

void BangleJSDevice::parseServices()
{
    qDebug() << Q_FUNC_INFO;

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

            if ( !service(uuid)) {
                addService(uuid, new QBLEService(uuid, path, this));
            }
        }
    }
    setConnectionState("authenticated");
}

void BangleJSDevice::initialise()
{
    setConnectionState("connected");
    parseServices();
}

void BangleJSDevice::onPropertiesChanged(QString interface, QVariantMap map, QStringList list)
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
                qDebug() << "DisConnected!";
                setConnectionState("disconnected");
            } else {
                setConnectionState("connected");
            }
        }
    }
}

void BangleJSDevice::authenticated(bool ready)
{
    qDebug() << Q_FUNC_INFO << ready;

    if (ready) {
        setConnectionState("authenticated");
    } else {
        setConnectionState("authfailed");
    }
}

AbstractFirmwareInfo *BangleJSDevice::firmwareInfo(const QByteArray &bytes)
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::navigationRunning(bool running)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress)
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration, int position)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::prepareFirmwareDownload(const AbstractFirmwareInfo *info)
{
    qDebug() << Q_FUNC_INFO;

}

void BangleJSDevice::startDownload()
{
    qDebug() << Q_FUNC_INFO;
}

void BangleJSDevice::refreshInformation()
{
    qDebug() << Q_FUNC_INFO;

}

QString BangleJSDevice::information(Info i) const
{
    qDebug() << Q_FUNC_INFO;

    return QString();
}

void BangleJSDevice::serviceEvent(uint8_t event)
{
    qDebug() << Q_FUNC_INFO;

}
