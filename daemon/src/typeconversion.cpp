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

QByteArray BEfromInt16(int val)
{
    return QByteArray(1, (val >> 8) & 0xff) + QByteArray(1, (val & 0xff));
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

    if (adjustForTZ) {
        QDateTime local = dt.toLocalTime();
        utcOffset = local.offsetFromUtc(); //QTimeZone::systemTimeZone().offsetFromUtc(local);
    }
    qDebug() << "UTC offset it " << utcOffset;

    ret += char((utcOffset / (60 * 60)) * 4);
    //ret += char(1);

    qDebug() << "converted date" << dt << "to" << ret;
    return ret;
}

QDateTime rawBytesToDateTime(const QByteArray &value, bool honorDeviceTimeOffset) {
    qDebug() << "Date length is " << value.length() << value.toHex() << honorDeviceTimeOffset;

    if (value.length() >= 7) {
        int year = TypeConversion::toUint16(value[0], value[1]);

        QTimeZone tz(0);

        QDateTime timestamp(QDate(
                                year,
                                (value[2] & 0xff),
                                value[3] & 0xff),
                            QTime(
                                value[4] & 0xff,
                                value[5] & 0xff,
                                value[6] & 0xff),
                            tz);


        //force timezone to UTC always
        if (value.length() > 7 && honorDeviceTimeOffset) {
            tz = QTimeZone(value[7] * 15 * 60);
            timestamp.setTimeZone(tz);
        } else {
            timestamp.setTimeSpec(Qt::LocalTime);
        }

        qDebug() << "Watch timezone is:" << tz;
        qDebug() << "System timezone is" << QTimeZone::systemTimeZone() << QTimeZone::systemTimeZoneId();
        qDebug() << "Time:" << timestamp;

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

uint16_t toUint16(char val1, char val2) {
    return (val1 & 0xff) | ((val2 & 0xff) << 8);
}

uint16_t toUint16(const QByteArray &arr, int &idx) {
    uint16_t ret = toUint16(arr[idx], arr[idx+1]);
    idx +=2;
    return ret;
}

uint32_t toUint32(char val1, char val2, char val3, char val4) {
    return (val1 & 0xff) | ((val2 & 0xff) << 8) | ((val3 & 0xff) << 16) | ((val4 & 0xff) << 24);
}

uint32_t toUint32(const QByteArray &arr, int &idx) {
    uint32_t ret = toUint32(arr[idx], arr[idx+1], arr[idx+2], arr[idx+3]);
    idx +=4;
    return ret;
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

int32_t toInt32(char val1, char val2, char val3, char val4) {
    union {
        unsigned char a[4];
        int32_t val;
    } number;

    number.a[0] = val1;
    number.a[1] = val2;
    number.a[2] = val3;
    number.a[3] = val4;

    return number.val;
}

int64_t toInt64(char val1, char val2, char val3, char val4, char val5, char val6, char val7, char val8) {
    union {
        unsigned char a[8];
        int64_t val;
    } number;

    number.a[0] = val1;
    number.a[1] = val2;
    number.a[2] = val3;
    number.a[3] = val4;
    number.a[4] = val5;
    number.a[5] = val6;
    number.a[6] = val7;
    number.a[7] = val8;

    return number.val;
}
}
