#include "huami2021chunkedencoder.h"


Huami2021ChunkedEncoder::Huami2021ChunkedEncoder(QBLECharacteristic *characteristic, bool force2021Protocol, int mtu) : m_characteristic(characteristic), m_force202Protocl(force2021Protocol), m_mtu(mtu)
{

}

void Huami2021ChunkedEncoder::setEncryptionParameters(int encryptedSequenceNumber, QByteArray sharedSessionKey)
{
    m_encryptedSequenceNumber = encryptedSequenceNumber;
    m_sharedSessionKey = sharedSessionKey;
}

void Huami2021ChunkedEncoder::write(short type, QByteArray data, bool extendedFlags, bool encrypt)
{

}
