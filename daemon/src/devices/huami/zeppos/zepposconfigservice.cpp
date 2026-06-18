#include "zepposconfigservice.h"

ZeppOsConfigService::ZeppOsConfigService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x000a;
}

QString ZeppOsConfigService::name() const
{
    return "config";
}

void ZeppOsConfigService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload.toHex();
}

void ZeppOsConfigService::setAllDayHeartRateInterval(uint8_t minutes)
{
    QByteArray cmd;
    cmd += char(CMD_SET);
    cmd += char(GROUP_HEALTH);
    cmd += char(GROUP_HEALTH_VERSION);
    cmd += char(0x00); // reserved
    cmd += char(0x01); // arg count
    cmd += char(ARG_HEART_RATE_ALL_DAY_MONITORING);
    cmd += char(TYPE_BYTE);
    cmd += char(minutes);

    qDebug() << Q_FUNC_INFO << "minutes=" << minutes << cmd.toHex();

    write(cmd);
}

void ZeppOsConfigService::setSleepHeartRateDetection(bool enabled)
{
    QByteArray cmd;
    cmd += char(CMD_SET);
    cmd += char(GROUP_HEALTH);
    cmd += char(GROUP_HEALTH_VERSION);
    cmd += char(0x00); // reserved
    cmd += char(0x01); // arg count
    cmd += char(ARG_HEART_RATE_SLEEP_SUPPORT);
    cmd += char(TYPE_BOOL);
    cmd += (enabled ? char(0x01) : char(0x00));

    qDebug() << Q_FUNC_INFO << "enabled=" << enabled << cmd.toHex();

    write(cmd);
}

void ZeppOsConfigService::setFitnessGoalSteps(uint32_t steps)
{
    QByteArray cmd;
    cmd += char(CMD_SET);
    cmd += char(GROUP_HEALTH);
    cmd += char(GROUP_HEALTH_VERSION);
    cmd += char(0x00); // reserved
    cmd += char(0x01); // arg count
    cmd += char(ARG_FITNESS_GOAL_STEPS);
    cmd += char(TYPE_INT);
    cmd += char(steps & 0xff);
    cmd += char((steps >> 8) & 0xff);
    cmd += char((steps >> 16) & 0xff);
    cmd += char((steps >> 24) & 0xff);

    qDebug() << Q_FUNC_INFO << "steps=" << steps << cmd.toHex();

    write(cmd);
}
