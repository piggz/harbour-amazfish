#include "typeconversion.h"
#include <QTimeZone>

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

QDateTime rawBytesToDateTime(const QByteArray &value, bool honorDeviceTimeOffset) {
    if (value.length() >= 7) {
        int year = TypeConversion::toUint16(value[0], value[1]);
        QDateTime timestamp(QDate(
                    year,
                    (value[2] & 0xff) - 1,
                value[3] & 0xff),
                QTime(
                value[4] & 0xff,
                value[5] & 0xff,
                value[6] & 0xff)
                );


        if (value.length() > 7) {
            QTimeZone tz(value[7] * 15 * 60 * 1000);
            timestamp.setTimeZone(tz);
        }
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

}
