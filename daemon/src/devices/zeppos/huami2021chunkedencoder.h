#ifndef HUAMI2_21CHUNKEDENCODER_H
#define HUAMI2_21CHUNKEDENCODER_H

#include "qblecharacteristic.h"

class Huami2021ChunkedEncoder
{
public:
    Huami2021ChunkedEncoder(QBLECharacteristic *characteristic, bool force2021Protocol, int mtu = 23);

    void setEncryptionParameters(int encryptedSequenceNumber, QByteArray sharedSessionKey);
    void write(short type, QByteArray data, bool extendedFlags, bool encrypt);

private:
    QBLECharacteristic *m_characteristic;
    bool m_force202Protocl = false;
    int m_mtu = 23;
    int m_writeHandle = 0;

    int m_encryptedSequenceNumber = 0;
    QByteArray m_sharedSessionKey;

    int calcCrc32(QByteArray data);

};

#endif // HUAMI2_21CHUNKEDENCODER_H
