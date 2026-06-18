#ifndef ZEPPOSCONFIGSERVICE_H
#define ZEPPOSCONFIGSERVICE_H

#include "abstractzepposservice.h"

class ZeppOsConfigService : public AbstractZeppOsService
{
public:
    static const uint8_t CMD_SET = 0x05;

    static const uint8_t GROUP_HEALTH = 0x08;
    static const uint8_t GROUP_HEALTH_VERSION = 0x02;

    static const uint8_t ARG_HEART_RATE_ALL_DAY_MONITORING = 0x01;
    static const uint8_t ARG_HEART_RATE_SLEEP_SUPPORT = 0x11;
    static const uint8_t ARG_FITNESS_GOAL_STEPS = 0x52;

    static const uint8_t TYPE_BOOL = 0x0b;
    static const uint8_t TYPE_BYTE = 0x10;
    static const uint8_t TYPE_INT = 0x03;

    ZeppOsConfigService(ZeppOSDevice *device);

    QString name() const override;
    void handlePayload(const QByteArray &payload) override;

    void setAllDayHeartRateInterval(uint8_t minutes);
    void setSleepHeartRateDetection(bool enabled);
    void setFitnessGoalSteps(uint32_t steps);
};

#endif // ZEPPOSCONFIGSERVICE_H
