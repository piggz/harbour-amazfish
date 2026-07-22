#ifndef ZEPPOSCONFIGSERVICE_H
#define ZEPPOSCONFIGSERVICE_H

#include "abstractzepposservice.h"

#include <QMap>

class ZeppOsConfigService : public AbstractZeppOsService
{
public:
    static constexpr uint8_t CMD_CAPABILITIES_REQUEST  = 0x01;
    static constexpr uint8_t CMD_CAPABILITIES_RESPONSE = 0x02;
    static constexpr uint8_t CMD_REQUEST               = 0x03;
    static constexpr uint8_t CMD_RESPONSE              = 0x04;
    static constexpr uint8_t CMD_SET                   = 0x05;
    static constexpr uint8_t CMD_ACK                   = 0x06;

    static constexpr uint8_t GROUP_HEALTH = 0x08;
    static constexpr uint8_t GROUP_HEALTH_VERSION = 0x02;

    static constexpr uint8_t ARG_HEART_RATE_ALL_DAY_MONITORING = 0x01;
    static constexpr uint8_t ARG_HEART_RATE_SLEEP_SUPPORT = 0x11;
    static constexpr uint8_t ARG_FITNESS_GOAL_STEPS = 0x52;

    static constexpr uint8_t TYPE_SHORT = 0x01;
    static constexpr uint8_t TYPE_INT   = 0x03;
    static constexpr uint8_t TYPE_BOOL  = 0x0b;
    static constexpr uint8_t TYPE_BYTE  = 0x10;

    ZeppOsConfigService(ZeppOSDevice *device);

    QString name() const override;
    void handlePayload(const QByteArray &payload) override;
    void initialize() override;

    void setAllDayHeartRateInterval(char minutes);
    void setSleepHeartRateDetection(bool enabled);
    void setFitnessGoalSteps(uint32_t steps);

private:
    void handleCapabilitiesResponse(const QByteArray &payload);
    void handleConfigResponse(const QByteArray &payload);
    void requestConfig(uint8_t group);

    QMap<uint8_t, uint8_t> m_groupVersions;
    bool m_sentFitnessGoal = false;
};

#endif // ZEPPOSCONFIGSERVICE_H
