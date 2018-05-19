#include "qbledevice.h"

QBLEDevice::QBLEDevice(QObject *parent) : QObject(parent)
{
}

void QBLEDevice::setDevicePath(const QString &path)
{
    qDebug() << "QBLEDevice::setDevicePath";

    m_devicePath = path;
    m_deviceInterface = new QDBusInterface("org.bluez", m_devicePath, "org.bluez.Device1", QDBusConnection::systemBus());

    qDebug() << m_deviceInterface->isValid();
    QStringList argumentMatch;
    argumentMatch << "org.bluez.Device1";

    QDBusConnection::systemBus().connect("org.bluez", m_devicePath, "org.freedesktop.DBus.Properties","PropertiesChanged", argumentMatch, QString(),
                                         this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
}

void QBLEDevice::pair()
{
    qDebug() << "QBLEDevice::pair";

    m_deviceInterface->asyncCall("Pair");
}

void QBLEDevice::connectToDevice()
{
    qDebug() << "QBLEDevice::connectToDevice";

    m_deviceInterface->asyncCall("Connect");
}

void QBLEDevice::disconnectFromDevice()
{
    qDebug() << "QBLEDevice::disconnectFromDevice";

    m_deviceInterface->call("Disconnect");
}

QBLEService* QBLEDevice::service(const QString &uuid)
{
    return m_serviceMap.value(uuid, nullptr);
}

QString QBLEDevice::devicePath() const
{
    return m_devicePath;
}

void QBLEDevice::addService(const QString &uuid, QBLEService *service)
{
    m_serviceMap[uuid] = service;
}

void QBLEDevice::onPropertiesChanged(const QString &interface, const QVariantMap &map, const QStringList &list)
{
    emit propertiesChanged(interface, map, list);
}

QVariant QBLEDevice::deviceProperty(const char *name) const
{
    if (m_deviceInterface) {
        return m_deviceInterface->property(name);
    }
    return QVariant();
}
