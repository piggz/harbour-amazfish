#ifndef HUAMIINITOPERATION2_21_H
#define HUAMIINITOPERATION2_21_H

#include "abstractoperation.h"
#include "huami2021chunkedencoder.h"

class HuamiInitOperation2021 : public AbstractOperation
{
public:
    HuamiInitOperation2021(bool needsAuth, uint8_t authFlags, uint8_t cryptFlags);

    void handleData(const QByteArray &data) override;
    void start(QBLEService *service) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;



private:

    uint8_t m_privateEC[24];
    uint8_t m_publicEC[48];
    uint8_t m_remotePublicEC[48];
    uint8_t m_remoteRandom[16];
    QByteArray m_sharedEC;
    uint8_t m_finalSharedSessionAES[16];

    Huami2021ChunkedEncoder *m_encoder = nullptr;

    void generateKeyPair();
};

#endif // HUAMIINITOPERATION2_21_H
