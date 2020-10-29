#include "miband2service.h"
#include "qaesencryption.h"
#include "amazfishconfig.h"

const char* MiBand2Service::UUID_SERVICE_MIBAND2  = "0000fee1-0000-1000-8000-00805f9b34fb";
const char* MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH = "00000009-0000-3512-2118-0009af100700";

MiBand2Service::MiBand2Service(const QString &path, char authByte, char cryptByte, bool requireAuthKey, QObject *parent) : QBLEService(UUID_SERVICE_MIBAND2, path, parent)
{
    qDebug() << "MiBand2Service::MiBand2Service";

    m_authByte = authByte;
    m_cryptByte = cryptByte;
    m_requiresAuthKey = requireAuthKey;
    qDebug() << "auth:" << (int)m_authByte << "crypt:" << (int)m_cryptByte << "req:" << m_requiresAuthKey;

    connect(this, &QBLEService::characteristicChanged, this, &MiBand2Service::characteristicChanged);
}

void MiBand2Service::initialise(bool firstTime)
{
    if (firstTime) {
        qDebug() << "Sending auth key";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHAR_TO_BYTEARRAY(&AUTH_SEND_KEY) + UCHAR_TO_BYTEARRAY(&m_authByte)/* + getSecretKey()*/);
    } else {
        qDebug() << "Writing request for auth number";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH , requestAuthNumber());
    }
}

void MiBand2Service::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << "Mi2Band Changed:" << characteristic << value;

    if (value[0] == RESPONSE && value[1] == AUTH_SEND_KEY && ((value[2] & SUCCESS) == SUCCESS || value[2] == 0x06) ) {
        qDebug() << "Received initial auth success, requesting random auth number";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHAR_TO_BYTEARRAY(&AUTH_REQUEST_RANDOM_AUTH_NUMBER) + UCHAR_TO_BYTEARRAY(&m_authByte) + UCHAR_TO_BYTEARRAY(&AUTH_REQUEST_RANDOM_AUTH_NUMBER));
    } else  if (value[0] == RESPONSE && (value[1] & 0x0f) == AUTH_REQUEST_RANDOM_AUTH_NUMBER && value[2] == SUCCESS) {
        qDebug() << "Received random auth number, sending encrypted auth number";
        //writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, QByteArray(&AUTH_SEND_ENCRYPTED_AUTH_NUMBER, 1) + QByteArray(&m_authByte, 1) + handleAesAuth(value.mid(3, 17), getSecretKey()));
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, QByteArray(1, (char)(m_cryptByte | AUTH_SEND_ENCRYPTED_AUTH_NUMBER)) + QByteArray(&m_authByte, 1) + handleAesAuth(value.mid(3, 17), getSecretKey()));
    } else  if (value[0] == RESPONSE && (value[1] & 0x0f) == AUTH_SEND_ENCRYPTED_AUTH_NUMBER && value[2] == SUCCESS) {
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
    auto deviceAuthKey = AmazfishConfig::instance()->deviceAuthKey();
    if (m_requiresAuthKey && !deviceAuthKey.isEmpty()) {
        return QByteArray::fromHex(deviceAuthKey.toLocal8Bit());
    }
    return AUTH_SECRET_KEY;
}

QByteArray MiBand2Service::requestAuthNumber() {
    qDebug() << "Crypt Byte:" << m_cryptByte;
    if (m_cryptByte == 0x00) {
        return UCHAR_TO_BYTEARRAY(&AUTH_REQUEST_RANDOM_AUTH_NUMBER) + UCHAR_TO_BYTEARRAY(&m_authByte);
    } else {
        return QByteArray(1, (char)(m_cryptByte | AUTH_REQUEST_RANDOM_AUTH_NUMBER)) + QByteArray(&m_authByte, 1) + QByteArray(1, 0x02)+ QByteArray(1, 0x01)+ QByteArray(1, 0x00);
    }
}
