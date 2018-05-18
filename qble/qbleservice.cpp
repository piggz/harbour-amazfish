#include "qbleservice.h"
#include <QtXml/QtXml>
QBLEService::QBLEService(const QString &uuid,const QString &path,  QObject *parent) : QObject(parent)
{
    m_serviceUUID = uuid;
    m_servicePath = path;
    m_serviceInterface = new QDBusInterface("org.bluez", m_servicePath, "org.bluez.GattService1", QDBusConnection::systemBus());
    introspect();
}

void QBLEService::characteristicChangedInt(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicChangedInt:" << characteristic << value.size() << value.toHex();
    emit characteristicChanged(characteristic, value);
}

void QBLEService::characteristicReadInt(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicReadInt:" << characteristic << value.size() << value.toHex();
    emit characteristicRead(characteristic, value);
}

void QBLEService::characteristicWrittenInt(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicWrittenInt:" << characteristic << value.size() << value.toHex();
    emit characteristicRead(characteristic, value);
}

void QBLEService::descriptorWrittenInt(const QString &descriptor, const QByteArray &value)
{
    qDebug() << "descriptorWrittenInt:" << descriptor << value.size() << value.toHex();
    emit descriptorWritten(descriptor, value);
}


void QBLEService::enableNotification(const QString &c)
{
    qDebug() << "Starting notify for " << c;

    QBLECharacteristic *ch = characteristic(c);

    if (ch) {
        ch->startNotify();
    } else {
        qDebug() << "Unable to get characteristic";
    }
}

void QBLEService::disableNotification(const QString &c)
{
    qDebug() << "Stopping notify for " << c;

    QBLECharacteristic *ch = characteristic(c);

    if (ch) {
        ch->stopNotify();
    } else {
        qDebug() << "Unable to get characteristic";
    }
}

QString QBLEService::serviceUUID() const
{
    return m_serviceUUID;
}

void QBLEService::writeValue(const QString &c, const QByteArray &value)
{
    qDebug() << "Writing to " << c << ":" << value.toHex();
    QBLECharacteristic *ch = characteristic(c);

    if (ch) {
        ch->writeValue(value);
    } else {
        qDebug() << "Unable to get characteristic";
    }
}

QByteArray QBLEService::readValue(const QString &c)
{
    qDebug() << "Reading from " << c;

    QBLECharacteristic *ch = characteristic(c);

    if (ch) {
        return ch->readValue();
    } else {
        qDebug() << "Unable to get characteristic";
    }
    return QByteArray();
}

void QBLEService::introspect()
{
    QDBusInterface miIntro("org.bluez", m_servicePath, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = miIntro.call("Introspect");
    QDomDocument doc;

    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    qDebug() << nodes.count() << "nodes";

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("char")) {
            QString path = m_servicePath + "/" + nodeName;
            QDBusInterface charInterface("org.bluez", path, "org.bluez.GattCharacteristic1", QDBusConnection::systemBus(), 0);
            m_characteristicMap[charInterface.property("UUID").toString()] = new QBLECharacteristic(path, this);
        }
    }

    foreach(QBLECharacteristic* c, m_characteristicMap.values()) {
        connect(c, &QBLECharacteristic::characteristicChanged, this, &QBLEService::characteristicChangedInt);
        connect(c, &QBLECharacteristic::characteristicRead, this, &QBLEService::characteristicReadInt);
    }

    qDebug() << "Introspect:characteristics:" << m_characteristicMap.keys();
}

void QBLEService::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    emit propertiesChanged(interface, map, list);
}

QBLECharacteristic *QBLEService::characteristic(const QString &c) const
{
    return m_characteristicMap.value(c, nullptr);
}

void QBLEService::readAsync(const QString &c) const
{
    qDebug() << "Async reading from " << c;

    QBLECharacteristic *ch = characteristic(c);

    if (ch) {
        ch->readAsync();
    } else {
        qDebug() << "Unable to get characteristic";
    }
}
