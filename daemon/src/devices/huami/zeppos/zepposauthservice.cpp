#include "zepposauthservice.h"

#include "ecdh/ecdh.h"
#include "typeconversion.h"
#include "amazfishconfig.h"
#include "huami/zepposdevice.h"
#include "mibandservice.h"

#include <Qt-AES/qaesencryption.h>
#include <random>

uint8_t getRandomUint8() {
    static std::random_device rd;  // Non-deterministic random device
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint16_t> dist(0, 255); // Use uint16_t to cover full uint8_t range

    return static_cast<uint8_t>(dist(gen));
}

ZeppOsAuthService::ZeppOsAuthService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    qDebug() << Q_FUNC_INFO;
    m_endpoint = 0x0082;
}

QString ZeppOsAuthService::name() const
{
    return "Auth";
}

void ZeppOsAuthService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload.toHex();

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
        qDebug() << "Secret Key: " << secretKey.toHex();

        QByteArray f;
        f.resize(16);
        for (int i = 0; i < 16; i++) {
            f[i] = m_finalSharedSessionAES[i];
        }
        qDebug() << "Shared Session Key: " << f.toHex();

        m_device->setEncryptionParameters(encryptedSequenceNumber, f);

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

            m_device->writeToChunked2021(m_endpoint, command, false);
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

void ZeppOsAuthService::startAuthentication()
{
    qDebug() << Q_FUNC_INFO;

    QByteArray sendPubkeyCommand;

    generateKeyPair();

    sendPubkeyCommand += QByteArray(1, 0x04);
    sendPubkeyCommand += QByteArray(1, 0x02);
    sendPubkeyCommand += QByteArray(1, 0x00);
    sendPubkeyCommand += QByteArray(1, 0x02);
    sendPubkeyCommand += UCHARARR_TO_BYTEARRAY(m_publicEC);

    m_device->writeToChunked2021(m_endpoint, sendPubkeyCommand, false);
}

#if 0
bool ZeppOsAuthService::characteristicChanged(const QString &characteristic, const QByteArray &value)
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

#endif

void ZeppOsAuthService::generateKeyPair()
{
    for (unsigned int i = 0; i < 24; ++i) {
        m_privateEC[i] = getRandomUint8();
    }
    ecdh_generate_keys(m_publicEC, m_privateEC);
}

void ZeppOsAuthService::debugArrayPrint(const QString &name, uint8_t *arr, int size)
{
    qDebug() << name << ":";
#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
    qDebug() << "FIXME";
#else
    QDebug dbg(QtDebugMsg);
    for (int i = 0; i < size; i++) {
        dbg << Qt::hex << arr[i] << (i < size ? ':' : '\n');
    }
#endif
}

