#ifndef ZEPPOSAUTHSERVICE_H
#define ZEPPOSAUTHSERVICE_H

#include "abstractzepposservice.h"
#include "qbleservice.h"

class ZeppOsAuthService : public AbstractZeppOsService
{
public:
    static constexpr uint8_t CMD_PUB_KEY = 0x04;
    static constexpr uint8_t CMD_SESSION_KEY = 0x05;

    ZeppOsAuthService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &data) override;
    QString name() const override;

    void startAuthentication();

private:

    uint8_t m_privateEC[24];
    uint8_t m_publicEC[48];
    uint8_t m_remotePublicEC[48];
    uint8_t m_remoteRandom[16];
    uint8_t m_sharedEC[48];
    uint8_t m_finalSharedSessionAES[16];
    QBLEService *m_service = nullptr;
    bool m_done = false;

    void generateKeyPair();
    void debugArrayPrint(const QString &name, uint8_t *arr, int size);
};

#endif // HUAMIINITOPERATION2_21_H
