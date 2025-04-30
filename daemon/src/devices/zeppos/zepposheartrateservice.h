#ifndef ZEPPOSHEARTRATESERVICE_H
#define ZEPPOSHEARTRATESERVICE_H

#include "zeppos/abstractzepposservice.h"

class ZeppOsHeartRateService : public AbstractZeppOsService
{
public:

    static const uint8_t CMD_REALTIME_SET = 0x04;
    static const uint8_t CMD_REALTIME_ACK = 0x05;
    static const uint8_t CMD_SLEEP = 0x06;

    static const uint8_t SLEEP_EVENT_FALL_ASLEEP = 0x01;
    static const uint8_t SLEEP_EVENT_WAKE_UP = 0x00;

    static const uint8_t REALTIME_MODE_STOP = 0x00;
    static const uint8_t REALTIME_MODE_START = 0x01;
    static const uint8_t REALTIME_MODE_CONTINUE = 0x02;

    ZeppOsHeartRateService(ZeppOSDevice *device);

    void handlePayload(const QByteArray &payload) override;
    QString name() const override;

    void enableRealtimeHeartRateMeasurement(bool enable, bool oneshot);
    void handleHeartRate(const QByteArray &value);

private:
    // Tracks whether realtime HR monitoring is already started, so we can just
    // send CONTINUE commands
    bool realtimeStarted = false;
    bool realtimeOneShot = false;

    int m_heartRate = 0;
};

#endif // ZEPPOSHEARTRATESERVICE_H
