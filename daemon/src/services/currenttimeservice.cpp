#include "currenttimeservice.h"
#include "typeconversion.h"

const char* CurrentTimeService::UUID_SERVICE_CURRENT_TIME = "00001805-0000-1000-8000-00805f9b34fb";
const char* CurrentTimeService::UUID_CHARACTERISTIC_CURRENT_TIME = "00002a2b-0000-1000-8000-00805f9b34fb";

CurrentTimeService::CurrentTimeService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_CURRENT_TIME, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}

void CurrentTimeService::setCurrentTime()
{
    QByteArray timeBytes;

    QDateTime now = QDateTime::currentDateTime();
    timeBytes += TypeConversion::fromInt16(now.date().year());
    timeBytes += TypeConversion::fromInt8(now.date().month());
    timeBytes += TypeConversion::fromInt8(now.date().day());
    timeBytes += TypeConversion::fromInt8(now.time().hour());
    timeBytes += TypeConversion::fromInt8(now.time().minute());
    timeBytes += TypeConversion::fromInt8(now.time().second());
    timeBytes += char(0); //day of week
    timeBytes += char(0); //fractions256
    timeBytes += char(0); //reason

    qDebug() << Q_FUNC_INFO << "Setting time to:" << now << timeBytes.toHex();
    writeValue(UUID_CHARACTERISTIC_CURRENT_TIME, timeBytes);
}

QDateTime CurrentTimeService::currentTime()
{
    QByteArray dateTime = readValue(UUID_CHARACTERISTIC_CURRENT_TIME);
    QDateTime dt = TypeConversion::rawBytesToDateTime(dateTime, false);

    qDebug() << Q_FUNC_INFO << "Read the watch date/time as" << dt;
    return dt;
}
