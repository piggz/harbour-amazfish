#include "typeconversion.h"
#include <QTimeZone>
#include <QDebug>

namespace TypeConversion {

QByteArray fromInt8(int val)
{
    return QByteArray(1, val & 0xff);
}

QByteArray fromInt16(int val)
{
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff));
}

QByteArray fromInt24(int val)
{
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff)) + QByteArray(1, ((val >> 16) & 0xff));
}

QByteArray fromInt32(int val)
{
    qDebug() << "Converting int32 to char" << val;
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff)) + QByteArray(1, ((val >> 16) & 0xff)) + QByteArray(1, ((val >> 24) & 0xff));
}

QByteArray fromInt64(long long int val)
{
    QByteArray ret = QByteArray(1, val & 0xff)
     + QByteArray(1, ((val >> 8) & 0xff))
     + QByteArray(1, ((val >> 16) & 0xff))
     + QByteArray(1, ((val >> 24) & 0xff))
     + QByteArray(1, ((val >> 32) & 0xff))
     + QByteArray(1, ((val >> 40) & 0xff))
     + QByteArray(1, ((val >> 48) & 0xff))
     + QByteArray(1, ((val >> 56) & 0xff));

    qDebug() << "Converting int64 to char" << val << " " << ret.toHex();

    return ret;
}


QByteArray dateTimeToBytes(const QDateTime &dt, int format, bool adjustForTZ)
{
    QByteArray ret;

    if (format == 0) {//short
        ret += fromInt16(dt.date().year());
        ret += fromInt8(dt.date().month());
        ret += fromInt8(dt.date().day());
        ret += fromInt8(dt.time().hour());
        ret += fromInt8(dt.time().minute());
    } else if (format == 1) { //long
        ret += fromInt16(dt.date().year());
        ret += fromInt8(dt.date().month());
        ret += fromInt8(dt.date().day());
        ret += fromInt8(dt.time().hour());
        ret += fromInt8(dt.time().minute());
        ret += fromInt8(dt.time().second());

    }

    //Tail of the data
    ret += QByteArray(1, char(0x00));

    //Timezone
    int utcOffset = 0;
    //Keep watch on UTC and convert when reading back
    //if (adjustForTZ) {
    //    utcOffset = QTimeZone::systemTimeZone().offsetFromUtc(dt);
    //}
    qDebug() << "UTC offset it " << utcOffset;

    ret += char((utcOffset / (60 * 60)) * 4);
    //ret += char(1);

    qDebug() << "converted date" << dt << "to" << ret;
    return ret;
}

QDateTime rawBytesToDateTime(const QByteArray &value, bool honorDeviceTimeOffset) {
    if (value.length() >= 7) {
        qDebug() << "Date length is " << value.length() << value;
        int year = TypeConversion::toUint16(value[0], value[1]);

        QTimeZone tz(0);

        //force timezone to UTC always
        //if (value.length() > 7 && honorDeviceTimeOffset) {
        //    tz = QTimeZone(value[7] * 15 * 60);
        //} /*else {
        //    timestamp.setTimeSpec(Qt::LocalTime);
        //}*/

        qDebug() << "Watch timezone is:" << tz;
        qDebug() << "System timezone is" << QTimeZone::systemTimeZone() << QTimeZone::systemTimeZoneId();

        QDateTime timestamp(QDate(
                                year,
                                (value[2] & 0xff),
                            value[3] & 0xff),
                QTime(
                    value[4] & 0xff,
                value[5] & 0xff,
                value[6] & 0xff),
                tz);
        /*
        QDateTime temp = QDateTime::fromString(timestamp.toString("yyyy-MM-dd hh:mm:ss"), "yyyy-MM-dd hh:mm:ss");
        qDebug() << "Timestamp: " << timestamp << timestamp.toString(Qt::ISODate);
        qDebug() << "Conveted timestamp" << temp << temp.toString(Qt::ISODate);

        QDateTime temp2 = temp.toTimeSpec( Qt::OffsetFromUTC);
        qDebug() << "Conveted timestamp" << temp2 << temp2.toString(Qt::ISODate);
        QDateTime temp3 = temp.toTimeSpec( Qt::UTC);
        qDebug() << "Conveted timestamp" << temp3 << temp3.toString(Qt::ISODate);

        QDateTime timestamp2(QDate(
                                 year,
                                 (value[2] & 0xff),
                             value[3] & 0xff),
                QTime(
                    value[4] & 0xff,
                value[5] & 0xff,
                value[6] & 0xff));
        qDebug() << "Timestamp2: " << timestamp2 << timestamp2.toString(Qt::ISODate);
        QDateTime temp4 = timestamp2.toTimeSpec( Qt::UTC);
        qDebug() << "Conveted timestamp4" << temp4 << temp3.toString(Qt::ISODate);
        */
        /*
        if (honorDeviceTimeOffset) {
            int offsetInHours = MiBandCoordinator.getDeviceTimeOffsetHours();
            if (offsetInHours != 0) {
                timestamp.add(Calendar.HOUR_OF_DAY,-offsetInHours);
            }
        }
        */
        return timestamp;
    }

    return QDateTime();
}

int toUint16(char val1, char val2) {
    return (val1 & 0xff) | ((val2 & 0xff) << 8);
}

int toUint32(char val1, char val2, char val3, char val4) {
    return (val1 & 0xff) | ((val2 & 0xff) << 8) | ((val3 & 0xff) << 16) | ((val4 & 0xff) << 24);
}

long toUnsigned(int unsignedInt) {
    return ((long) unsignedInt) & 0xffffffffL;
}
int toUnsigned(short value) {
    return value & 0xffff;
}

int toUnsigned(char value) {
    return value & 0xff;
}

int16_t toInt16(uint8_t val1, uint8_t val2) {
    return (int16_t)((val1 & 0xff) | ((val2 & 0xff) << 8));
}

uint16_t toUint16(char value) {
    return toUnsigned(value);
}
}
