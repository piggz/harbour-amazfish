#include "miband2service.h"
#include "qaesencryption.h"
#include "amazfishconfig.h"

const char* MiBand2Service::UUID_SERVICE_MIBAND2  = "0000fee1-0000-1000-8000-00805f9b34fb";
const char* MiBand2Service::UUID_CHARACTERISITIC_MIBAND2_AUTH = "00000009-0000-3512-2118-0009af100700";

MiBand2Service::MiBand2Service(const QString &path, char authByte, char cryptByte, bool requireAuthKey, QObject *parent) : QBLEService(UUID_SERVICE_MIBAND2, path, parent)
{
    qDebug() << Q_FUNC_INFO;

    m_authByte = authByte;
    m_cryptByte = cryptByte;
    m_requiresAuthKey = requireAuthKey;
    qDebug() << "auth:" << (int)m_authByte << "crypt:" << (int)m_cryptByte << "req:" << m_requiresAuthKey;

    connect(this, &QBLEService::characteristicChanged, this, &MiBand2Service::characteristicChanged, Qt::UniqueConnection);
}

void MiBand2Service::initialise(bool firstTime)
{
    if (firstTime) {
        qDebug() << Q_FUNC_INFO << "Sending auth key";
        qDebug() << AUTH_SEND_KEY <<  sizeof(AUTH_SEND_KEY) << sizeof(&AUTH_SEND_KEY) << UCHAR_TO_BYTEARRAY(AUTH_SEND_KEY);
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHAR_TO_BYTEARRAY(AUTH_SEND_KEY) + UCHAR_TO_BYTEARRAY(m_authByte) + getSecretKey());
    } else {
        qDebug() << Q_FUNC_INFO << "Writing request for auth number";
        //uint8_t start[2] = {0x01, 0x00};
        //writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHARARR_TO_BYTEARRAY(start));
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH , requestAuthNumber());
    }
}

void MiBand2Service::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << "Changed:" << characteristic << value;

    if (value[0] == RESPONSE && value[1] == AUTH_SEND_KEY && ((value[2] & SUCCESS) == SUCCESS || value[2] == 0x06) ) {
        qDebug() << "Received initial auth success, requesting random auth number";
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHAR_TO_BYTEARRAY(AUTH_REQUEST_RANDOM_AUTH_NUMBER) + UCHAR_TO_BYTEARRAY(m_authByte) + UCHAR_TO_BYTEARRAY(AUTH_REQUEST_RANDOM_AUTH_NUMBER));
    } else  if (value[0] == RESPONSE && (value[1] & 0x0f) == AUTH_REQUEST_RANDOM_AUTH_NUMBER && value[2] == SUCCESS) {
        qDebug() << "Received random auth number, sending encrypted auth number";
        uint8_t a = (m_cryptByte | AUTH_SEND_ENCRYPTED_AUTH_NUMBER);
        writeValue(UUID_CHARACTERISITIC_MIBAND2_AUTH, UCHAR_TO_BYTEARRAY(a) + UCHAR_TO_BYTEARRAY(m_authByte) + handleAesAuth(value.mid(3, 17), getSecretKey()));
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
    qDebug() << Q_FUNC_INFO << "Crypt Byte:" << m_cryptByte;
    if (m_cryptByte == 0x00) {
        return UCHAR_TO_BYTEARRAY(AUTH_REQUEST_RANDOM_AUTH_NUMBER) + UCHAR_TO_BYTEARRAY(m_authByte);
    } else {
        uint8_t req = (m_cryptByte | AUTH_REQUEST_RANDOM_AUTH_NUMBER);
        uint8_t suffix[3] = {0x02,0x01, 0x00};
        //uint8_t suffix[1] = {0x02};

        return UCHAR_TO_BYTEARRAY(req) + UCHAR_TO_BYTEARRAY(m_authByte) + UCHARARR_TO_BYTEARRAY(suffix);
    }
}
