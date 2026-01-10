#ifndef ZEPPOSSERVICESSERVICE_H
#define ZEPPOSSERVICESSERVICE_H

#include "huami/zeppos/abstractzepposservice.h"

class ZeppOsServicesService : public AbstractZeppOsService
{
public:

    static const uint8_t CMD_GET_LIST = 0x03;
    static const uint8_t CMD_RET_LIST = 0x04;

    ZeppOsServicesService(ZeppOSDevice *device);
    void handlePayload(const QByteArray &payload) override;
    QString name() const override;

    void requestServices();

private:
    void handleSupportedServices(QByteArray payload);

};

#endif // ZEPPOSSERVICESSERVICE_H
