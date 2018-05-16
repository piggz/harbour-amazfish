#include "qbleservice.h"

QBLEService::QBLEService(const QString &uuid, QObject *parent) : QObject(parent)
{
    m_serviceUUID = uuid;
}

void QBLEService::setPath(const QString &path)
{
    m_servicePath = path;
    m_serviceService = new QDBusInterface("org.bluez", m_servicePath, "org.bluez.GattService1", QDBusConnection::systemBus());
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
    qDebug() << "Enabling notification for " << c << " in " << m_servicePath;
//    QLowEnergyCharacteristic characteristic = service()->characteristic(QBluetoothUuid(c));
//    QLowEnergyDescriptor notificationDesc = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
//    if (notificationDesc.isValid()) {
//        service()->writeDescriptor(notificationDesc, QByteArray::fromHex("0100"));
//    } else {
//        qDebug() << "notification is invalid";
//    }
}

void QBLEService::disableNotification(const QString &c)
{
    qDebug() << "Disabling notification for " << c << " in " << m_servicePath;
//    QLowEnergyCharacteristic characteristic = service()->characteristic(QBluetoothUuid(c));
//    QLowEnergyDescriptor notificationDesc = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
//    if (notificationDesc.isValid()) {
//        service()->writeDescriptor(notificationDesc, QByteArray::fromHex("0000"));
//    } else {
//        qDebug() << "notification is invalid";
//    }
}

QString QBLEService::serviceUUID() const
{
    return m_serviceUUID;
}

void QBLEService::writeValue(const char *CHARACTERISTIC, const QByteArray &value)
{
        qDebug() << "Writing to " << CHARACTERISTIC << ":" << value.toHex();
//        service()->writeCharacteristic(service()->characteristic(QBluetoothUuid(QString(CHARACTERISTIC))) , value, QLowEnergyService::WriteWithResponse);

}

QVariant readCharacteristic(const QString &characteristic)
{
    return QVariant();
}
