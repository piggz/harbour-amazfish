#ifndef ZEPPOSTIMESERVICE_H
#define ZEPPOSTIMESERVICE_H

#include "abstractzepposservice.h"
#include "huami/zepposdevice.h"

class ZeppOsTimeService : public AbstractZeppOsService
{
public:    
    static const uint8_t CMD_CAPABILITIES_REQUEST = 0x01;
    static const uint8_t CMD_CAPABILITIES_RESPONSE = 0x02;
    static const uint8_t CMD_SET_TIME = 0x05;
    static const uint8_t CMD_SET_TIME_ACK = 0x06;
    static const uint8_t CMD_SET_DST = 0x07;
    static const uint8_t CMD_SET_DST_ACK = 0x08;

    ZeppOsTimeService(ZeppOSDevice *device);
    QString name() const override;
    void handlePayload(const QByteArray &payload) override;

    void setTime();

};

#endif // ZEPPOSTIMESERVICE_H
