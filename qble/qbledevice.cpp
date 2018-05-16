#include "qbledevice.h"

QBLEDevice::QBLEDevice(QObject *parent) : QObject(parent)
{
}

void QBLEDevice::connectToDevice(const QString &path)
{
    m_devicePath = path;
    m_deviceService = new QDBusInterface("org.bluez", m_devicePath, "org.bluez.Device1", QDBusConnection::systemBus());
}

void QBLEDevice::disconnectFromDevice()
{

}
