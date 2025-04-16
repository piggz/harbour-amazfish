#ifndef HUAMIINITOPERATION2_21_H
#define HUAMIINITOPERATION2_21_H

#include "abstractoperation.h"
#include "huami2020handler.h"
#include "huami2021chunkedencoder.h"
#include "huami2021chunkeddecoder.h"

class HuamiInitOperation2021 : public AbstractOperation, Huami2020Handler
{
public:
    HuamiInitOperation2021(bool needsAuth, uint8_t authFlags, uint8_t cryptFlags);

    void handleData(const QByteArray &data) override;
    bool handleMetaData(const QByteArray &data) override;
    void start(QBLEService *service) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

    void handle2021Payload(short type, const QByteArray &data) override;


private:

    uint8_t m_privateEC[24];
    uint8_t m_publicEC[48];
    uint8_t m_remotePublicEC[48];
    uint8_t m_remoteRandom[16];
    uint8_t m_sharedEC[48];
    uint8_t m_finalSharedSessionAES[16];
    QBLEService *m_service = nullptr;

    Huami2021ChunkedEncoder *m_encoder = nullptr;
    Huami2021ChunkedDecoder *m_decoder = nullptr;

    void generateKeyPair();
    void debugArrayPrint(const QString &name, uint8_t *arr, int size);
};

#endif // HUAMIINITOPERATION2_21_H
