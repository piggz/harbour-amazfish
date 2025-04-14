#include "huami2021chunkeddecoder.h"
#include <QDebug>
#include <qaesencryption.h>

Huami2021ChunkedDecoder::Huami2021ChunkedDecoder(bool force2021Protocal) : m_force2021Protocol(force2021Protocal)
{
    qDebug() << Q_FUNC_INFO;
}

void Huami2021ChunkedDecoder::setEncryptionParameters(QByteArray sharedSessionKey)
{
    m_sharedSessionKey = sharedSessionKey;
}

uint8_t Huami2021ChunkedDecoder::lastHandle()
{
    return m_lastHandle;
}

uint8_t Huami2021ChunkedDecoder::lastCount()
{
    return m_lastCount;
}

bool Huami2021ChunkedDecoder::decode(QByteArray data)
{
    qDebug() << Q_FUNC_INFO << data.toHex(':');

    int i = 0;
    if (data.at(i++) != 0x03) {
        qDebug()<< "Ignoring non-chunked payload";
        return false;
    }
    uint8_t flags = data[i++];
    qDebug() << "Flags:" << flags;

    bool encrypted = ((flags & 0x08) == 0x08);
    bool firstChunk = ((flags & 0x01) == 0x01);
    bool lastChunk = ((flags & 0x02) == 0x02);
    bool needsAck = ((flags & 0x04) == 0x04);

    if (m_force2021Protocol) {
        i++; // skip extended header
    }
    uint8_t handle = data[i++];
    if (m_currentHandle != 0 && m_currentHandle != handle) {
        qDebug() << "ignoring handle: " << handle << m_currentHandle;
        return false;
    }
    m_lastHandle = handle;
    m_lastCount = data[i++];

    if (firstChunk) { // beginning
        int full_length = (data[i++] & 0xff) | ((data[i++] & 0xff) << 8) | ((data[i++] & 0xff) << 16) | ((data[i++] & 0xff) << 24);
        m_currentLength = full_length;
        if (encrypted) {
            int encrypted_length = full_length + 8;
            int overflow = encrypted_length % 16;
            if (overflow > 0) {
                encrypted_length += (16 - overflow);
            }
            full_length = encrypted_length;
        }
        m_reassemblyBuffer.resize(full_length);
        m_currentType = (data[i++] & 0xff) | ((data[i++] & 0xff) << 8);
        m_currentHandle = handle;
    }

    //m_reassemblyBuffer.put(data, i, data.length() - i);
    for (int j = i; j < data.length() - i; j++) {
        m_reassemblyBuffer[j-i] = (data[j]);
    }
    if (lastChunk) { // end
        if (encrypted) {
            if (m_sharedSessionKey.isNull()) {
                // Should never happen
                qDebug() << "Got encrypted message, but there's no shared session key";
                m_currentHandle = 0;
                m_currentType = 0;
                return false;
            }

            QByteArray messageKey;
            messageKey.resize(16);
            for (int j = 0; j < 16; j++) {
                messageKey[j] = (m_sharedSessionKey[j] ^ handle);
            }

            m_reassemblyBuffer = QAESEncryption::Decrypt(QAESEncryption::AES_128, QAESEncryption::ECB, m_reassemblyBuffer, messageKey, QByteArray(), QAESEncryption::ZERO);
            m_reassemblyBuffer = m_reassemblyBuffer.mid(0, m_currentLength);
        }

        qDebug() << m_reassemblyBuffer;
        if (m_handler) {
            m_handler->handle2021Payload(m_currentType, m_reassemblyBuffer);
        }

        m_currentHandle = 0;
        m_currentType = 0;
    }

    qDebug() << Q_FUNC_INFO << needsAck;
    return needsAck;
}

void Huami2021ChunkedDecoder::setHuami2021Handler(Huami2020Handler *handler)
{
    m_handler = handler;
}

