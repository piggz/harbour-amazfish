#include "zeppostimeservice.h"
#include "typeconversion.h"


ZeppOsTimeService::ZeppOsTimeService(ZeppOSDevice *device) : AbstractZeppOsService(device, false)
{
    m_endpoint = 0x0047;
}

QString ZeppOsTimeService::name() const
{
    return "time";
}

void ZeppOsTimeService::handlePayload(const QByteArray &payload)
{
    qDebug() << Q_FUNC_INFO << payload.toHex();

    int version = 9;

    switch (payload[0]) {
    case CMD_CAPABILITIES_RESPONSE:
        version = payload[1] & 0xff;
        qDebug() << "Got time service version:", version;
        return;
    case CMD_SET_TIME_ACK:
        qDebug() << "Got set time ack, status:", payload[1]; // 1
        return;
    case CMD_SET_DST_ACK:
        qDebug() << "Got set DST ack, status:", payload[1]; // 1
        return;
    }

    qDebug() << "Unexpected time payload byte:", payload.toHex();
}

void ZeppOsTimeService::setTime()
{
    qDebug() << Q_FUNC_INFO;

    QByteArray timeBytes;

    QDateTime now = QDateTime::currentDateTime();
    timeBytes += CMD_SET_TIME;
    timeBytes += TypeConversion::fromInt16(now.date().year());
    timeBytes += TypeConversion::fromInt8(now.date().month());
    timeBytes += TypeConversion::fromInt8(now.date().day());
    timeBytes += TypeConversion::fromInt8(now.time().hour());
    timeBytes += TypeConversion::fromInt8(now.time().minute());
    timeBytes += TypeConversion::fromInt8(now.time().second());
    timeBytes += TypeConversion::fromInt8(now.date().dayOfWeek() - 1);
    timeBytes += TypeConversion::fromInt8((now.time().msec() / 1000 ) * 256);
    timeBytes += now.isDaylightTime() ? char(0x08) : char(0x00);

    // calculate timezone
    int offsetInSec = now.offsetFromUtc();
    timeBytes += char(offsetInSec/(15*60));

    qDebug() << Q_FUNC_INFO << "setting time to:" << now << ", tz: " << offsetInSec << timeBytes.toHex();

    write(timeBytes);
}
