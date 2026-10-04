#include "zepposconfigservice.h"
#include "amazfishconfig.h"
#include "typeconversion.h"

ZeppOsConfigService::ZeppOsConfigService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x000a;
}

QString ZeppOsConfigService::name() const
{
    return "config";
}

void ZeppOsConfigService::initialize()
{
    qDebug() << Q_FUNC_INFO;

    QByteArray data;
    data += char(CMD_CAPABILITIES_REQUEST);
    write(data);
}

void ZeppOsConfigService::handlePayload(const QByteArray &payload)
{
    switch (payload[0]) {
    case CMD_CAPABILITIES_RESPONSE:
        handleCapabilitiesResponse(payload);
        return;

    case CMD_ACK:
        qDebug() << "Configuration ACK, status =" << uint8_t(payload[1]);
        return;

    case CMD_RESPONSE:
        if (payload[1] != 1) {
            qDebug() << "Configuration response not success:" << uint8_t(payload[1]);
            return;
        }
        handleConfigResponse(payload);
        return;

    default:
        qDebug() << "Unexpected configuration payload byte" << QString::number(uint8_t(payload[0]), 16);
    }
}

void ZeppOsConfigService::handleCapabilitiesResponse(const QByteArray &payload)
{
    int version = payload[1] & 0xff;
    qDebug() << "Got config service version =" << version;
    if (version > 3) {
        qDebug() << "Unsupported config service version" << version;
        return;
    }

    int numGroups = payload[2] & 0xff;
    if (payload.length() != numGroups + 3) {
        qDebug() << "Unexpected config capabilities response length" << payload.length() << "for" << numGroups << "groups";
        return;
    }

    for (int i = 0; i < numGroups; i++) {
        uint8_t configGroup = payload[3 + i];
        qDebug() << "Got supported config group:" << QString::number(configGroup, 16);
        requestConfig(configGroup);
    }
}

void ZeppOsConfigService::requestConfig(uint8_t group)
{
    QByteArray cmd;
    cmd += char(CMD_REQUEST);
    cmd += char(0x01); // includeConstraints = true
    cmd += char(group);
    cmd += char(0x00); // num args = 0 (request all)

    qDebug() << Q_FUNC_INFO << "group =" << QString::number(group, 16) << cmd.toHex();

    write(cmd);
}

void ZeppOsConfigService::handleConfigResponse(const QByteArray &payload)
{
    // payload[0] = CMD_RESPONSE (already checked)
    // payload[1] = status (already checked)
    uint8_t configGroup = payload[2];
    uint8_t version = payload[3];

    // Store the group version for future use (e.g. HEALTH v1 vs v2 type selection)
    m_groupVersions[configGroup] = version;

    bool includesConstraints = payload[4] == 0x01;
    int numConfigs = payload[5] & 0xff;

    qDebug() << "Got" << numConfigs << "configs for group" << QString::number(configGroup, 16)
             << "version" << QString::number(version, 16) << "includesConstraints" << includesConstraints;

    // After receiving HEALTH configs, send the fitness goal if not yet sent
    if (configGroup == GROUP_HEALTH && !m_sentFitnessGoal) {
        m_sentFitnessGoal = true;
        setFitnessGoalSteps(AmazfishConfig::instance()->profileFitnessGoal());
    }
}

void ZeppOsConfigService::setAllDayHeartRateInterval(char minutes)
{
    QByteArray cmd;
    cmd += char(CMD_SET);
    cmd += char(GROUP_HEALTH);
    cmd += char(GROUP_HEALTH_VERSION);
    cmd += char(0x00); // reserved
    cmd += char(0x01); // arg count
    // TODO refactor the above in generic setConfig in upcoming settings

    minutes = minutes < 0 ? -1 : minutes; // Any negative value works for testing "smart"
    cmd += char(ARG_HEART_RATE_ALL_DAY_MONITORING);
    cmd += char(TYPE_BYTE);
    cmd += char(minutes);

    qDebug() << Q_FUNC_INFO << "minutes=" << (uint8_t)minutes << cmd.toHex();

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
    // Choose type based on HEALTH group version: v1 uses SHORT, v2+ uses INT
    uint8_t stepsType = TYPE_INT; // default for v2+
    if (m_groupVersions.contains(GROUP_HEALTH)) {
        uint8_t healthVersion = m_groupVersions[GROUP_HEALTH];
        if (healthVersion == 0x01) {
            stepsType = TYPE_SHORT;
        }
    }

    QByteArray cmd;
    cmd += char(CMD_SET);
    cmd += char(GROUP_HEALTH);
    cmd += char(GROUP_HEALTH_VERSION);
    cmd += char(0x00); // reserved
    cmd += char(0x01); // arg count
    cmd += char(ARG_FITNESS_GOAL_STEPS);
    cmd += char(stepsType);

    if (stepsType == TYPE_SHORT) {
        // v1: 2-byte little-endian uint16
        cmd += TypeConversion::fromInt16(steps);
    } else {
        // v2+: 4-byte little-endian uint32
        cmd += TypeConversion::fromInt32(steps);
    }

    qDebug() << Q_FUNC_INFO << "steps=" << steps << "type=" << QString::number(stepsType, 16) << cmd.toHex();

    write(cmd);
}
