#include "timeformatter.h"
#include <QObject>
#include <qmath.h>

/**
 * @brief TimeFormatter::formatHMS
 * Returns with a formatted string displaying the given time separated with ':'-s.
 * The return format depends on the time interval length, if not necessary it will
 * not display the hours (below 1 hour interval) and minutes (under 1 minute interval)
 *
 * @param hours
 * @param minutes
 * @param seconds
 * @return
 */
QString TimeFormatter::formatHMS_fromSeconds(const uint durationInSec)
{
    uint hours = durationInSec / (60*60);
    hours = qFloor(hours);
    uint minutes = (durationInSec - hours*60*60) / 60;
    minutes = qFloor(minutes);
    uint seconds = durationInSec - hours*60*60 - minutes*60;
    seconds = qFloor(seconds);
    return formatHMS(hours, minutes, seconds);
}

QString TimeFormatter::formatHMS(const uint hours, const uint minutes, const uint seconds)
{
    if(hours == 0)
    {
        if(minutes == 0)
        {
            return QObject::tr("%1s").arg(seconds);
        }
        return QObject::tr("%1m %2s")
                .arg(minutes)
                .arg(seconds, 2, 10, QLatin1Char('0'));
    }
    return QObject::tr("%1h %2m %3s")
            .arg(hours)
            .arg(minutes, 2, 10, QLatin1Char('0'))
            .arg(seconds, 2, 10, QLatin1Char('0'));
}
