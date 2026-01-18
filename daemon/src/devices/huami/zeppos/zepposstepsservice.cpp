#include "zepposstepsservice.h"
#include "typeconversion.h"

ZeppOsStepsService::ZeppOsStepsService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x0016;
}

QString ZeppOsStepsService::name() const
{
    return "Steps";
}

void ZeppOsStepsService::handlePayload(const QByteArray &payload)
{
    switch (payload[0]) {
    case CMD_REPLY:
        qDebug() << "Got steps reply, status = " <<  payload[1];
        if (payload.size() != 15) {
            qDebug() << "Unexpected steps reply payload length " << payload.size();
            return;
        }
        handleRealtimeSteps(payload.mid(2, 15));
        return;
    case CMD_ENABLE_REALTIME_ACK:
        qDebug() << "Band acknowledged realtime steps, status = " << payload[1] << " enabled = " << payload[2];
        return;
    case CMD_REALTIME_NOTIFICATION:
        qDebug() << "Got steps notification";
        if (payload.size() != 14) {
            qDebug() << "Unexpected realtime notification payload length {}", payload.size();
            return;
        }
        handleRealtimeSteps(payload.mid(1, 14));
        return;
    }

    qDebug() << "Unexpected steps payload byte " << payload[0];
}

void ZeppOsStepsService::enableRealtimeSteps(bool enable)
{
    qDebug() << Q_FUNC_INFO << enable;

    QByteArray cmd;
    cmd += CMD_ENABLE_REALTIME;
    cmd += (enable ? 0x01 : 0x00);

    write(cmd);
}

void ZeppOsStepsService::handleRealtimeSteps(const QByteArray value)
{
    if (value.size() != 13) {
        qDebug() << "Unrecognized realtime steps value: " << value.toHex();
        return;
    }

    int steps = TypeConversion::toUint16(value[1], value[2]);
    qDebug() << "realtime steps: " << steps;

    m_device->informationChanged(Amazfish::Info::INFO_STEPS, QString::number(steps));
}
