#ifndef ZEPPOSSTEPSSERVICE_H
#define ZEPPOSSTEPSSERVICE_H

#include "abstractzepposservice.h"
#include "zepposdevice.h"

class ZeppOsStepsService : public AbstractZeppOsService
{
public:

    static const uint8_t CMD_GET = 0x03;
    static const uint8_t CMD_REPLY = 0x04;
    static const uint8_t CMD_ENABLE_REALTIME = 0x05;
    static const uint8_t CMD_ENABLE_REALTIME_ACK = 0x06;
    static const uint8_t CMD_REALTIME_NOTIFICATION = 0x07;

    ZeppOsStepsService(ZeppOSDevice *device);

    QString name() const override;
    void handlePayload(const QByteArray &payload) override;

    void enableRealtimeSteps(bool enable);

private:
    void handleRealtimeSteps(const QByteArray data);

};

#endif // ZEPPOSSTEPSSERVICE_H
