#include "huamiinitoperation2021.h"

#include "ecdh/ecdh.h"
#include "mibandservice.h"
#include "amazfishconfig.h"
#include "typeconversion.h"
#include <Qt-AES/qaesencryption.h>
#include <iostream>

uint8_t getRandomUint8() {
    static std::random_device rd;  // Non-deterministic random device
    static std::mt19937 gen(rd()); // Mersenne Twister RNG
    static std::uniform_int_distribution<uint16_t> dist(0, 255); // Use uint16_t to cover full uint8_t range

    return static_cast<uint8_t>(dist(gen));
}

HuamiInitOperation2021::HuamiInitOperation2021(bool needsAuth, uint8_t authFlags, uint8_t cryptFlags)
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
#if 0
    huamiSupport.enableNotifications(builder, true);
    builder.add(new SetDeviceStateAction(getDevice(), GBDevice.State.INITIALIZING, getContext()));
    // get random auth number
    generateKeyPair();
    byte[] sendPubkeyCommand = new byte[48 + 4];
    sendPubkeyCommand[0] = 0x04;
    sendPubkeyCommand[1] = 0x02;
    sendPubkeyCommand[2] = 0x00;
    sendPubkeyCommand[3] = 0x02;
    System.arraycopy(publicEC, 0, sendPubkeyCommand, 4, 48);
    //testAuth();
    huami2021ChunkedEncoder.write(builder, Huami2021Service.CHUNKED2021_ENDPOINT_AUTH, sendPubkeyCommand, true, false);
#endif

    QByteArray sendPubkeyCommand;
    m_service = service;

    m_encoder = new Huami2021ChunkedEncoder(service->characteristic(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_WRITE), true);
    m_decoder = new Huami2021ChunkedDecoder(true);
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
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex(':');

#if 0
    UUID characteristicUUID = characteristic.getUuid();
    if (!HuamiService.UUID_CHARACTERISTIC_CHUNKEDTRANSFER_2021_READ.equals(characteristicUUID)) {
        LOG.info("Unhandled characteristic changed: " + characteristicUUID);
        return super.onCharacteristicChanged(gatt, characteristic);
    }

    byte[] value = characteristic.getValue();
    if (value.length <= 1 || value[0] != 0x03) {
        // Not chunked
        return super.onCharacteristicChanged(gatt, characteristic);
    }

    final boolean needsAck = huami2021ChunkedDecoder.decode(value);
    if (needsAck) {
        huamiSupport.sendChunkedAck();
    }

    return true;
#endif

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
        ack.append(UCHAR_TO_BYTEARRAY(0x04));
        ack.append(UCHAR_TO_BYTEARRAY(0x00));
        ack.append(UCHAR_TO_BYTEARRAY(m_decoder->lastHandle()));
        ack.append(UCHAR_TO_BYTEARRAY(0x01));
        ack.append(UCHAR_TO_BYTEARRAY(m_decoder->lastCount()));

        if (m_service) {
            QBLECharacteristic *c = m_service->characteristic(MiBandService::UUID_CHARACTERISTIC_MIBAND_2021_CHUNKED_CHAR_READ);
            if (c) {
                c->writeValue(ack);
            }
        }
    }

    return false;
}

void HuamiInitOperation2021::handle2021Payload(short type, const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << type << payload.toHex(':');

    if (type != MiBandService::CHUNKED2021_ENDPOINT_AUTH) {
        qDebug() << "Unandles message type";
        return;
    }

    if (payload[0] == MiBandService::RESPONSE && payload[1] == 0x04 && payload[2] == MiBandService::SUCCESS && payload.length() == 67) {
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
        qDebug() << "Shared Session Key: " << f.toHex(':');

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
            command.append(0x05);
            command.append(encryptedRandom1);
            command.append(encryptedRandom2);

            m_encoder->write(MiBandService::CHUNKED2021_ENDPOINT_AUTH, command, true, false);
            //huamiSupport.performImmediately(builder);
        } else {
            qDebug() << "Random lengths not 16:" << encryptedRandom1.length() << encryptedRandom2.length();
        }
    } else {
        qDebug() << "Unexpected payload";
    }
#if 0
    if (type != Huami2021Service.CHUNKED2021_ENDPOINT_AUTH) {
                this.huamiSupport.handle2021Payload(type, payload);
                return;
            }

            if (payload[0] == RESPONSE && payload[1] == 0x04 && payload[2] == SUCCESS) {
                LOG.debug("Got remote random + public key");
                // Received remote random (16 bytes) + public key (48 bytes)

                System.arraycopy(payload, 3, remoteRandom, 0, 16);
                System.arraycopy(payload, 19, remotePublicEC, 0, 48);
                sharedEC = ECDH_B163.ecdh_generate_shared(privateEC, remotePublicEC);
                int encryptedSequenceNumber = (sharedEC[0] & 0xff) | ((sharedEC[1] & 0xff) << 8) | ((sharedEC[2] & 0xff) << 16) | ((sharedEC[3] & 0xff) << 24);

                byte[] secretKey = getSecretKey();
                for (int i = 0; i < 16; i++) {
                    finalSharedSessionAES[i] = (byte) (sharedEC[i + 8] ^ secretKey[i]);
                }

                LOG.debug("Shared Session Key: {}", GB.hexdump(finalSharedSessionAES));
                huami2021ChunkedEncoder.setEncryptionParameters(encryptedSequenceNumber, finalSharedSessionAES);
                huami2021ChunkedDecoder.setEncryptionParameters(finalSharedSessionAES);

                try {
                    byte[] encryptedRandom1 = CryptoUtils.encryptAES(remoteRandom, secretKey);
                    byte[] encryptedRandom2 = CryptoUtils.encryptAES(remoteRandom, finalSharedSessionAES);
                    if (encryptedRandom1.length == 16 && encryptedRandom2.length == 16) {
                        byte[] command = new byte[33];
                        command[0] = 0x05;
                        System.arraycopy(encryptedRandom1, 0, command, 1, 16);
                        System.arraycopy(encryptedRandom2, 0, command, 17, 16);
                        TransactionBuilder builder = createTransactionBuilder("Sending double encryted random to device");
                        huami2021ChunkedEncoder.write(builder, Huami2021Service.CHUNKED2021_ENDPOINT_AUTH, command, true, false);
                        huamiSupport.performImmediately(builder);
                    }
                } catch (Exception e) {
                    LOG.error("AES encryption failed", e);
                }
            } else if (payload[0] == RESPONSE && payload[1] == 0x05 && payload[2] == SUCCESS) {
                LOG.debug("Auth Success");

                try {
                    TransactionBuilder builder = createTransactionBuilder("Authenticated, now initialize phase 2");
                    builder.add(new SetDeviceStateAction(getDevice(), GBDevice.State.INITIALIZING, getContext()));
                    builder.setCallback(null); // remove init operation as the callback
                    huamiSupport.enableFurtherNotifications(builder, true);
                    huamiSupport.setCurrentTimeWithService(builder);
                    huamiSupport.requestDeviceInfo(builder);
                    huamiSupport.phase2Initialize(builder);
                    huamiSupport.phase3Initialize(builder);
                    huamiSupport.setInitialized(builder);
                    huamiSupport.performImmediately(builder);
                } catch (Exception e) {
                    LOG.error("failed initializing device", e);
                }
            } else if (payload[0] == RESPONSE && payload[1] == 0x05 && payload[2] == 0x25) {
                LOG.error("Authentication failed, disconnecting");
                GB.toast(getContext(), R.string.authentication_failed_check_key, Toast.LENGTH_LONG, GB.WARN);
                final GBDevice device = getDevice();
                if (device != null) {
                    GBApplication.deviceService(device).disconnect();
                }
            } else {
                LOG.warn("Unhandled auth payload: {}", GB.hexdump(payload));
            }
#endif

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

