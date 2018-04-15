#include "miband2service.h"
#include "qaesencryption.h"

const char* MiBand2Service::UUID_SERVICE_MIBAND2  = "{0000FEE1-0000-1000-8000-00805f9b34fb}";
const char* MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH = "{00000009-0000-3512-2118-0009af100700}";

MiBand2Service::MiBand2Service(QObject *parent) : BipService("{0000FEE1-0000-1000-8000-00805f9b34fb}", parent)
{
    connect(this, &BipService::characteristicChanged, this, &MiBand2Service::characteristicChanged);
//    connect(this, &BipService::readyChanged, this, &MiBand2Service::serviceReady);
}

void MiBand2Service::initialise()
{
    service()->writeCharacteristic(service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISITIC_MIBAND2_AUTH))) , QByteArray(&AUTH_REQUEST_RANDOM_AUTH_NUMBER, 1) + QByteArray(&AUTH_BYTE, 1), QLowEnergyService::WriteWithoutResponse);
}

void MiBand2Service::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &value)
{
    qDebug() << "Mi2Band Changed:" << characteristic.uuid() << "(" << characteristic.name() << "):" << value;

    if (value[0] == AUTH_RESPONSE && value[1] == AUTH_REQUEST_RANDOM_AUTH_NUMBER && value[2] == AUTH_SUCCESS) {
        qDebug() << "Received random auth number, sending encrypted auth number";
        service()->writeCharacteristic(service()->characteristic(QBluetoothUuid(QString(UUID_CHARACTERISITIC_MIBAND2_AUTH))), QByteArray(&AUTH_SEND_ENCRYPTED_AUTH_NUMBER, 1) + QByteArray(&AUTH_BYTE, 1) + handleAesAuth(value.mid(3, 17), AUTH_SECRET_KEY), QLowEnergyService::WriteWithoutResponse);
    } else  if (value[0] == AUTH_RESPONSE && value[1] == AUTH_SEND_ENCRYPTED_AUTH_NUMBER && value[2] == AUTH_SUCCESS) {
        qDebug() << "Authenticated";
        emit authenticated();
    }
}

QByteArray MiBand2Service::handleAesAuth(QByteArray data, QByteArray secretKey)
{
    return QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, data, secretKey, QByteArray(), QAESEncryption::ZERO);
}
