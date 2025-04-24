#include "huamiinitoperation2021.h"

#include "ecdh/ecdh.h"
#include "mibandservice.h"
#include "amazfishconfig.h"
#include "typeconversion.h"
#include <Qt-AES/qaesencryption.h>

uint8_t getRandomUint8() {
    static std::random_device rd;  // Non-deterministic random device
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint16_t> dist(0, 255); // Use uint16_t to cover full uint8_t range

    return static_cast<uint8_t>(dist(gen));
}

HuamiInitOperation2021::HuamiInitOperation2021(bool needsAuth, uint8_t authFlags, uint8_t cryptFlags, HuamiDevice *device, Huami2021ChunkedEncoder *encoder, Huami2021ChunkedDecoder *decoder) : m_device(device), m_encoder(encoder), m_decoder(decoder)
{
    qDebug() << Q_FUNC_INFO;
}

void HuamiInitOperation2021::handleData(const QByteArray &data)
{

}

bool HuamiInitOperation2021::handleMetaData(const QByteArray &data)
{
    return false;
}

void HuamiInitOperation2021::start(QBLEService *service)
{
    qDebug() << Q_FUNC_INFO;

    QByteArray sendPubkeyCommand;
    m_service = service;

    m_decoder->setHuami2021Handler(this);

    generateKeyPair();

    sendPubkeyCommand += QByteArray(1, 0x04);
    sendPubkeyCommand += QByteArray(1, 0x02);
    sendPubkeyCommand += QByteArray(1, 0x00);
    sendPubkeyCommand += QByteArray(1, 0x02);
    sendPubkeyCommand += UCHARARR_TO_BYTEARRAY(m_publicEC);
    m_encoder->write(MiBandService::CHUNKED2021_ENDPOINT_AUTH, sendPubkeyCommand, true, false);
}

bool HuamiInitOperation2021::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex();

    if (characteristic != MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ) {
        qDebug() << "Unhandled characteristic:" << characteristic;
        return false;
    }

    if (value.length() <= 1 || value[0] != 0x03) {
        //Not chunked
        return false;
    }

    bool needsAck = m_decoder->decode(value);
    if (needsAck) {
        qDebug() << "Sending ACK to device";
        QByteArray ack;
        ack += CMD_PUB_KEY;
        ack += UCHARVAL_TO_BYTEARRAY(0x00);
        ack += m_decoder->lastHandle();
        ack += 0x01;
        ack += m_decoder->lastCount();

        if (m_service) {
            QBLECharacteristic *c = m_service->characteristic(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
            if (c) {
                c->writeValue(ack);
            }
        }
    }

    return m_done;
}

void HuamiInitOperation2021::handle2021Payload(short type, const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << type << payload.toHex();

    if (type != MiBandService::CHUNKED2021_ENDPOINT_AUTH) {
        qDebug() << "Unandles message type";
        return;
    }

    if (payload[0] == MiBandService::RESPONSE && payload[1] == CMD_PUB_KEY && payload[2] == MiBandService::SUCCESS && payload.length() == 67) {
        qDebug() << "Got remote random + public key";

        for (int i = 0; i < 16; i++) {
            m_remoteRandom[i] = payload[i + 3];
        }

        for (int i = 0; i < 48; i++) {
            m_remotePublicEC[i] = payload[i + 19];
        }

        qDebug() << "Generating shared secret";
        qDebug() << ecdh_shared_secret(m_privateEC, m_remotePublicEC, m_sharedEC);

        debugArrayPrint("m_remoteRandom", m_remoteRandom, 16);
        debugArrayPrint("m_remotePublicEC", m_remotePublicEC, 48);
        debugArrayPrint("m_privateEC", m_privateEC, 24);
        debugArrayPrint("m_publicEC", m_publicEC, 48);
        debugArrayPrint("m_sharedEC", m_sharedEC, 48);

        uint32_t encryptedSequenceNumber = TypeConversion::toUint32(m_sharedEC[0], m_sharedEC[1], m_sharedEC[2], m_sharedEC[3]);
        QByteArray secretKey = QByteArray::fromHex(AmazfishConfig::instance()->deviceAuthKey().toLocal8Bit());
        for (int i = 0; i < 16; i++) {
            m_finalSharedSessionAES[i] = (m_sharedEC[i + 8] ^ secretKey[i]);
        }
        qDebug() << "Secret Key: " << secretKey.toHex(';');

        QByteArray f;
        f.resize(16);
        for (int i = 0; i < 16; i++) {
            f[i] = m_finalSharedSessionAES[i];
        }
        qDebug() << "Shared Session Key: " << f.toHex();

        m_encoder->setEncryptionParameters(encryptedSequenceNumber, f);
        m_decoder->setEncryptionParameters(f);

        QByteArray r;
        r.resize(16);
        for (int i = 0; i < 16; i++) {
            r[i] = m_remoteRandom[i];
        }

        QByteArray encryptedRandom1 = QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, r, secretKey, QByteArray(), QAESEncryption::ZERO);
        QByteArray encryptedRandom2 = QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, r, f, QByteArray(), QAESEncryption::ZERO);

        if (encryptedRandom1.length() == 16 && encryptedRandom2.length() == 16) {
            QByteArray command;
            command += CMD_SESSION_KEY;
            command += encryptedRandom1;
            command += encryptedRandom2;

            m_encoder->write(MiBandService::CHUNKED2021_ENDPOINT_AUTH, command, true, false);
            //huamiSupport.performImmediately(builder);
        } else {
            qDebug() << "Random lengths not 16:" << encryptedRandom1.length() << encryptedRandom2.length();
        }
    } else if (payload[0] == MiBandService::RESPONSE && payload[1] == CMD_SESSION_KEY && payload[2] == MiBandService::SUCCESS) {
        qDebug() << "Auth success";
        if (m_device) {
            m_device->authenticated(true);
        }
        m_done = true;
    } else {
        qDebug() << "Unexpected payload";
    }
}

void HuamiInitOperation2021::generateKeyPair()
{
    for (unsigned int i = 0; i < 24; ++i) {
        m_privateEC[i] = getRandomUint8();
    }
    ecdh_generate_keys(m_publicEC, m_privateEC);
}

void HuamiInitOperation2021::debugArrayPrint(const QString &name, uint8_t *arr, int size)
{
    qDebug() << name << ":";
    QDebug dbg(QtDebugMsg);
    for (int i = 0; i < size; i++) {
        dbg << Qt::hex << arr[i] << (i < size ? ':' : '\n');
    }
}

