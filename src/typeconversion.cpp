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
    return QByteArray(1, val & 0xff) + QByteArray(1, ((val >> 8) & 0xff)) + QByteArray(1, ((val >> 16) & 0xff)) + QByteArray(1, ((val >> 24) & 0xff));
}

QByteArray dateTimeToBytes(const QDateTime &dt, int format)
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
    int utcOffset = QTimeZone::systemTimeZone().offsetFromUtc(dt);
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
        QDateTime timestamp(QDate(
                                year,
                                (value[2] & 0xff),
                            value[3] & 0xff),
                QTime(
                    value[4] & 0xff,
                value[5] & 0xff,
                value[6] & 0xff)
                );


        qDebug() <<  timestamp;

        if (value.length() > 7) {
            QTimeZone tz(value[7] * 15 * 60);
            qDebug() << tz;
            timestamp.setTimeZone(tz);
        } else {
            timestamp.setTimeSpec(Qt::LocalTime);
        }
        qDebug() <<  timestamp;
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

}
