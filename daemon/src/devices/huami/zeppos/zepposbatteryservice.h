#ifndef ZEPPOSBATTERYSERVICE_H
#define ZEPPOSBATTERYSERVICE_H

#include "abstractzepposservice.h"

class ZeppOsBatteryService : public AbstractZeppOsService
{
public:
    static const uint8_t BATTERY_REQUEST = 0x03;
    static const uint8_t BATTERY_REPLY = 0x04;

    explicit ZeppOsBatteryService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &payload) override;
    QString name() const override;
private:

};

#endif // ZEPPOSBATTERYSERVICE_H
