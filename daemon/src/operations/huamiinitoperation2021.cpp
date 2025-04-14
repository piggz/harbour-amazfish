#include "huamiinitoperation2021.h"

#include "ecdh/ecdh.h"
#include "mibandservice.h"

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

void HuamiInitOperation2021::handle2021Payload(short type, const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO << type << data.toHex(':');

}

void HuamiInitOperation2021::generateKeyPair()
{
    for (unsigned int i = 0; i < 24; ++i) {
        m_privateEC[i] = getRandomUint8();
    }
    ecdh_generate_keys(m_publicEC, m_privateEC);
}

