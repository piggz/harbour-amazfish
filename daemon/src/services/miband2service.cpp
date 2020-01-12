#include "miband2service.h"
#include "qaesencryption.h"

const char* MiBand2Service::UUID_SERVICE_MIBAND2  = "0000fee1-0000-1000-8000-00805f9b34fb";
const char* MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH = "00000009-0000-3512-2118-0009af100700";

MiBand2Service::MiBand2Service(const QString &path, char authByte, QObject *parent) : QBLEService(UUID_SERVICE_MIBAND2, path, parent)
{
    qDebug() << "MiBand2Service::MiBand2Service";

    m_authByte = authByte;

    connect(this, &QBLEService::characteristicChanged, this, &MiBand2Service::characteristicChanged);
//    connect(this, &BipService::readyChanged, this, &MiBand2Service::serviceReady);
}

void MiBand2Service::initialise(bool firstTime)
{
    if (firstTime) {
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, QByteArray(&AUTH_SEND_KEY, 1) + QByteArray(&m_authByte, 1) + getSecretKey());
    } else {
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH , QByteArray(&AUTH_REQUEST_RANDOM_AUTH_NUMBER, 1) + QByteArray(&m_authByte, 1));
    }
}

void MiBand2Service::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "Mi2Band Changed:" << characteristic << value;



    if (value[0] == RESPONSE && value[1] == AUTH_SEND_KEY && (value[2] & SUCCESS) == SUCCESS) {
        qDebug() << "Received initial auth success, requesting random auth number";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, QByteArray(&AUTH_REQUEST_RANDOM_AUTH_NUMBER, 1) + QByteArray(&AUTH_BYTE, 1));
    } else  if (value[0] == RESPONSE && value[1] == AUTH_REQUEST_RANDOM_AUTH_NUMBER && value[2] == SUCCESS) {
        qDebug() << "Received random auth number, sending encrypted auth number";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, QByteArray(&AUTH_SEND_ENCRYPTED_AUTH_NUMBER, 1) + QByteArray(&m_authByte, 1) + handleAesAuth(value.mid(3, 17), getSecretKey()));
    } else  if (value[0] == RESPONSE && value[1] == AUTH_SEND_ENCRYPTED_AUTH_NUMBER && (value[2] == SUCCESS || value[2] == 0x06)) { //TODO not sure about 0x06
        qDebug() << "Authenticated";
        emit authenticated(true);
    } else {
        qDebug() << "Unexpected data";
        emit authenticated(false);
    }

}

QByteArray MiBand2Service::handleAesAuth(QByteArray data, QByteArray secretKey)
{
    return QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, data, secretKey, QByteArray(), QAESEncryption::ZERO);
}

QByteArray MiBand2Service::getSecretKey()
{
    QByteArray authKey = AUTH_SECRET_KEY;
    if (!m_settings.value("/uk/co/piggz/amazfish/device/authkey").toString().isEmpty()) {
        authKey = QByteArray::fromHex(m_settings.value("/uk/co/piggz/amazfish/device/authkey").toString().toLocal8Bit());
    }
    return authKey;
}
