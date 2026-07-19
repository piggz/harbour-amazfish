#include "huamidatasource.h"

static const long MIN_SESSION_LENGTH = 5 * 60;
static const long MAX_WAKE_PHASE_LENGTH = 2 * 60 * 60;
static const int TYPE_LIGHT_SLEEP = 9;
static const int TYPE_DEEP_SLEEP = 11;
static const int TYPE_NO_CHANGE = 0;
static const int TYPE_IGNORE = 10;

int kind(int k)
{
    static int lastRealKind;
    k = k & 0xf; //just look at top 4 bits
    if (k == TYPE_IGNORE || k == TYPE_NO_CHANGE) {
        return lastRealKind;
    }
    lastRealKind = k;
    return k;
}

HuamiDataSource::HuamiDataSource()
{
    qDebug() << Q_FUNC_INFO;
}

QList<DataSource::SleepSession> HuamiDataSource::calculateSleep(const QDate &day)
{
    // qDebug() << "Calculating sleep for " << day;
    QDateTime startDate = QDateTime(day, QTime(12, 00)); //In localtime
    QDateTime endDate = startDate.addDays(1);
    QList<DataSource::SleepSession> sessions;

    long previousSample = 0;
    QDateTime sleepStart;
    QDateTime sleepEnd;
    long lightSleepDuration = 0;
    long deepSleepDuration = 0;
    long durationSinceLastSleep = 0;

    QString qry = "SELECT timestamp, raw_kind, raw_intensity FROM mi_band_activity WHERE timestamp >= " +
            QString::number(startDate.toMSecsSinceEpoch() / 1000) + " AND timestamp <= '" +
            QString::number(endDate.toMSecsSinceEpoch() / 1000) +   "' ORDER BY timestamp ASC";

    // qDebug() << qry;

    if (m_conn && m_conn->isDatabaseUsed()) {
        KDbCursor *curs = m_conn->executeQuery(KDbEscapedString(qry));

        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {

                    qint64 d = curs->value(0).toLongLong();
                    int k = kind(curs->value(1).toInt());

                    if (isSleep(k)) {
                        if (!sleepStart.isValid()) {
                            sleepStart.setMSecsSinceEpoch(d * 1000);
                            qDebug() << "Setting sleep start to " << d << sleepStart;
                        }
                        sleepEnd.setMSecsSinceEpoch(d * 1000);
                        durationSinceLastSleep = 0;
                    }
                    if (previousSample != 0) {
                        long durationSinceLastSample = d - previousSample;
                        if (k == TYPE_LIGHT_SLEEP) {
                            lightSleepDuration += durationSinceLastSample;
                        } else if (k == TYPE_DEEP_SLEEP) {
                            deepSleepDuration += durationSinceLastSample;
                        } else {
                            durationSinceLastSleep += durationSinceLastSample;
                            if (sleepStart.isValid() && durationSinceLastSleep > MAX_WAKE_PHASE_LENGTH) {
                                if (lightSleepDuration + deepSleepDuration > MIN_SESSION_LENGTH) {
                                    SleepSession ss;
                                    ss.sleepStart = sleepStart;
                                    ss.sleepEnd = sleepEnd;
                                    ss.lightSleepDuration = lightSleepDuration;
                                    ss.deepSleepDuration = deepSleepDuration;
                                    sessions << ss;
                                    // qDebug() << "Sleep Session" << ss.sleepStart << ss.sleepEnd << ss.lightSleepDuration << ss.deepSleepDuration;

                                }
                                sleepStart = QDateTime();
                                sleepEnd = QDateTime();
                                lightSleepDuration = 0;
                                deepSleepDuration = 0;
                            }
                        }
                    }
                    previousSample = d;
                    curs->moveNext();
                }
                if (lightSleepDuration + deepSleepDuration > MIN_SESSION_LENGTH) {
                    SleepSession ss;
                    ss.sleepStart = sleepStart;
                    ss.sleepEnd = sleepEnd;
                    ss.lightSleepDuration = lightSleepDuration;
                    ss.deepSleepDuration = deepSleepDuration;
                    sessions << ss;
                }
            }
            m_conn->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    }

    return sessions;
}


bool HuamiDataSource::isSleep(int kind)
{
    int k = (int)kind & 0xf;
    return k == TYPE_DEEP_SLEEP || k == TYPE_LIGHT_SLEEP;
}