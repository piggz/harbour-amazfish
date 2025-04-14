#ifndef HUAMI2_21CHUNKEDDECODER_H
#define HUAMI2_21CHUNKEDDECODER_H

#include "huami2020handler.h"
#include <QByteArray>

class Huami2021ChunkedDecoder
{
public:
    Huami2021ChunkedDecoder(bool force2020Protocol);

    void setEncryptionParameters(QByteArray sharedSessionKey);
    uint8_t lastHandle();
    uint8_t lastCount();
    bool decode(QByteArray data);

    void setHuami2021Handler(Huami2020Handler *handler);

private:
    bool m_force2021Protocol = false;
    QByteArray m_sharedSessionKey;

    uint8_t m_lastHandle = 0;
    uint8_t m_lastCount = 0;
    uint8_t m_currentHandle = 0;
    int m_currentType = 0;
    int m_currentLength = 0;
    QByteArray m_reassemblyBuffer;
    Huami2020Handler *m_handler = nullptr;
};

#endif // HUAMI2_21CHUNKEDDECODER_H
