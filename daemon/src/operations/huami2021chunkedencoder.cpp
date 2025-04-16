#include "huami2021chunkedencoder.h"
#include "qbleservice.h"
#include <zlib.h>
#include <qaesencryption.h>

Huami2021ChunkedEncoder::Huami2021ChunkedEncoder(QBLECharacteristic *characteristic, bool force2021Protocol, int mtu) : m_characteristic(characteristic), m_force202Protocl(force2021Protocol), m_mtu(mtu)
{
    qDebug() << Q_FUNC_INFO;
}

void Huami2021ChunkedEncoder::setEncryptionParameters(int encryptedSequenceNumber, QByteArray sharedSessionKey)
{
    qDebug() << Q_FUNC_INFO << encryptedSequenceNumber << sharedSessionKey.toHex(':');
    m_encryptedSequenceNumber = encryptedSequenceNumber;
    m_sharedSessionKey = sharedSessionKey;
}

void Huami2021ChunkedEncoder::write(short type, QByteArray data, bool extendedFlags, bool encrypt)
{
    qDebug() << Q_FUNC_INFO << type << data.toHex(':') << extendedFlags << encrypt;
    if (!m_characteristic) {
        qDebug() << "No characteristic";
        return;
    }

    m_writeHandle++;
    int remaining = data.length();
    int length = data.length();
    uint8_t count = 0;
    int header_size = 10;

    if (extendedFlags) {
        header_size++;
    }

    if (extendedFlags && encrypt) {
        QByteArray messageKey;
        messageKey.reserve(16);
        for (int i = 0; i < 16; i++) {
            messageKey[i] = (char) (m_sharedSessionKey[i] ^ m_writeHandle);
        }
        int encrypted_length = length + 8;
        int overflow = encrypted_length % 16;
        if (overflow > 0) {
            encrypted_length += (16 - overflow);
        }

        QByteArray encryptable_payload;
        encryptable_payload.append(data);
        encryptable_payload.append(UCHAR_TO_BYTEARRAY((m_encryptedSequenceNumber & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((m_encryptedSequenceNumber >> 8) & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((m_encryptedSequenceNumber >> 16) & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((m_encryptedSequenceNumber >> 24) & 0xff)));

        m_encryptedSequenceNumber++;

        int checksum = calcCrc32(encryptable_payload);

        encryptable_payload.append(UCHAR_TO_BYTEARRAY((checksum & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((checksum >> 8) & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((checksum >> 16) & 0xff)));
        encryptable_payload.append(UCHAR_TO_BYTEARRAY(((checksum >> 24) & 0xff)));

        remaining = encrypted_length;
        data = QAESEncryption::Crypt(QAESEncryption::AES_128, QAESEncryption::ECB, encryptable_payload, messageKey, QByteArray(), QAESEncryption::ZERO);
    }

    qDebug() << Q_FUNC_INFO << "Chunking data:" << data.toHex(':');

    while (remaining > 0) {
        int MAX_CHUNKLENGTH = m_mtu - 3 - header_size;
        int copybytes = std::min(remaining, MAX_CHUNKLENGTH);
        QByteArray chunk;
        chunk.resize(copybytes + header_size);

        uint8_t flags = 0;
        if (encrypt) {
            flags |= 0x08;
        }
        if (count == 0) {
            flags |= 0x01;
            int i = 4;
            if (extendedFlags) {
                i++;
            }
            chunk[i++] = (length & 0xff);
            chunk[i++] = ((length >> 8) & 0xff);
            chunk[i++] = ((length >> 16) & 0xff);
            chunk[i++] = ((length >> 24) & 0xff);
            chunk[i++] = (type & 0xff);
            chunk[i] = ((type >> 8) & 0xff);
        }
        if (remaining <= MAX_CHUNKLENGTH) {
            flags |= 0x06; // last chunk?
        }
        chunk[0] = 0x03;
        chunk[1] = flags;
        if (extendedFlags) {
            chunk[2] = 0;
            chunk[3] = m_writeHandle;
            chunk[4] = count;
        } else {
            chunk[2] = m_writeHandle;
            chunk[3] = count;
        }


        for (uint i = 0; i < copybytes; i++) {
            chunk[header_size + i] = data[data.length() - remaining + i];
        }

        m_characteristic->writeValue(chunk);
        remaining -= copybytes;
        header_size = 4;

        if (extendedFlags) {
            header_size++;
        }

        count++;
    }
}

int Huami2021ChunkedEncoder::calcCrc32(QByteArray data)
{
    uint32_t crc = crc32(0L, Z_NULL, 0);

    char *bytes = data.data();
    int len = data.length();

    for (int i = 0; i < len; ++i)
    {
        crc = crc32(crc, (unsigned char*)bytes + i, 1);
    }
    return crc;
}
