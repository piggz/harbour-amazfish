#include "bluezadapter.h"
#include <QtXml/QtXml>

BluezAdapter::BluezAdapter(QObject *parent) : QObject(parent)
{
    qDebug() << "BluezAdapter::BluezAdapter";
}

void BluezAdapter::setAdapterPath(const QString &path)
{
    qDebug() << "BluezAdapter::setAdapterPath:" << path;

    m_adapterPath = path;
    m_deviceInterface = new QDBusInterface("org.bluez", m_adapterPath, "org.bluez.Adapter1", QDBusConnection::systemBus());
}

void BluezAdapter::startDiscovery()
{
    qDebug() << "BluezAdapter::startDiscovery";

    if (!m_deviceInterface && !m_adapterPath.isEmpty()) {
        setAdapterPath(m_adapterPath);
    }

    QDBusMessage reply = m_deviceInterface->call("StartDiscovery");
    qDebug() << "BluezAdapter::StartDiscovery:" << reply.errorMessage();
}

void BluezAdapter::stopDiscovery()
{
    qDebug() << "BluezAdapter::stopDiscovery";

    if (!m_deviceInterface && !m_adapterPath.isEmpty()) {
        setAdapterPath(m_adapterPath);
    }

    QDBusMessage reply = m_deviceInterface->call("StopDiscovery");
    qDebug() << "BluezAdapter::StopDiscovery:" <<  reply.errorMessage();
}

QString BluezAdapter::matchDevice(const QString &match)
{
    qDebug() << "BluezAdapter::matchDevice:looking for:" << match;

    QDBusInterface adapterIntro("org.bluez", m_adapterPath, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("dev_")) {
            QString path = m_adapterPath + "/" + nodeName;

            QDBusInterface devInterface("org.bluez", path, "org.bluez.Device1", QDBusConnection::systemBus(), 0);

            qDebug() << "Found:" << devInterface.property("Name");

            if (devInterface.property("Name").toString().contains(match)) {
                return path;
            }
        }
    }
    return QString();
}

bool BluezAdapter::deviceIsValid(const QString &match)
{
    QDBusInterface adapterIntro("org.bluez", m_adapterPath, "org.freedesktop.DBus.Introspectable", QDBusConnection::systemBus(), 0);
    QDBusReply<QString> xml = adapterIntro.call("Introspect");

    QDomDocument doc;
    doc.setContent(xml.value());

    QDomNodeList nodes = doc.elementsByTagName("node");

    for (int x = 0; x < nodes.count(); x++)
    {
        QDomElement node = nodes.at(x).toElement();
        QString nodeName = node.attribute("name");

        if (nodeName.startsWith("dev_")) {
            QString path = m_adapterPath + "/" + nodeName;

            if (path == match)
                return true;
        }
    }

    return false;
}
