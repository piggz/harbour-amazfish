#include "bipservice.h"

BipService::BipService(const QString &uuid, QObject *parent) : QObject(parent)
{
    m_serviceUUID = uuid;
}

void BipService::setController(QLowEnergyController *controller)
{
    m_controller = controller;
}

void BipService::connectToService()
{
    if (m_controller) {
        m_ready = false;
        m_service = m_controller->createServiceObject(QBluetoothUuid(m_serviceUUID), this);
        if (m_service) {
            connect(m_service, &QLowEnergyService::stateChanged, this, &BipService::serviceStateChanged);
            connect(m_service, &QLowEnergyService::characteristicChanged, this, &BipService::characteristicChangedInt);
            connect(m_service, &QLowEnergyService::characteristicRead, this, &BipService::characteristicReadInt);
            connect(m_service, &QLowEnergyService::characteristicWritten, this, &BipService::characteristicWrittenInt);

            connect(m_service, &QLowEnergyService::descriptorWritten, this, &BipService::descriptorWrittenInt);

            connect(m_service, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                [=](QLowEnergyService::ServiceError newError){
                qDebug() << "Service error" << newError;
            });


            m_service->discoverDetails();
        } else {
            qDebug() << "Service not found";
        }
    } else {
        qDebug() << "No controller set";
    }
}

void BipService::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    switch (s) {
    case QLowEnergyService::DiscoveringServices:
        qDebug() << "Discovering services...";
        break;
    case QLowEnergyService::ServiceDiscovered:
    {

        qDebug() << "Service secondary services for " << m_service->serviceUuid();
        Q_FOREACH(QBluetoothUuid s, m_service->includedServices()) {
            qDebug() << "-Secondary Service:" << s.toString();
        }

        qDebug() << "Service characteristics:";
        Q_FOREACH(QLowEnergyCharacteristic c, m_service->characteristics()) {
            qDebug() << "-Characteristic:" << c.uuid() << c.name();
        }
        m_ready = true;
        emit readyChanged(m_ready);

        break;
    }
    default:
        //nothing for now
        break;
    }
}

void BipService::characteristicChangedInt(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicChangedInt:" << characteristic.uuid() << value.size() << value.toHex();
    emit characteristicChanged(characteristic, value);
}

void BipService::characteristicReadInt(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicReadInt:" << characteristic.uuid() << value.size() << value.toHex();
    emit characteristicRead(characteristic, value);
}

void BipService::characteristicWrittenInt(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "characteristicWrittenInt:" << characteristic.uuid() << value.size() << value.toHex();
    //emit characteristicRead(characteristic, value);
}

void BipService::descriptorWrittenInt(const QLowEnergyDescriptor &descriptor, const QByteArray &value)
{
    qDebug() << "descriptorWrittenInt:" << descriptor.name() << descriptor.uuid() << value.size() << value.toHex();
    emit descriptorWritten(descriptor, value);
}

bool BipService::ready() const
{
    return m_ready;
}

QLowEnergyService *BipService::service() const
{
    return m_service;
}

void BipService::enableNotification(const QString &c)
{
    qDebug() << "Enabling notification for " << c << " in " << serviceUUID();
    QLowEnergyCharacteristic characteristic = service()->characteristic(QBluetoothUuid(c));
    QLowEnergyDescriptor notificationDesc = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (notificationDesc.isValid()) {
        service()->writeDescriptor(notificationDesc, QByteArray::fromHex("0100"));
    } else {
        qDebug() << "notification is invalid";
    }
}

QString BipService::serviceUUID() const
{
    return m_serviceUUID;
}
