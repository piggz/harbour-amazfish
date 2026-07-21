#include "zepposdatasource.h"
#include "typeconversion.h"


ZeppOSDataSource::ZeppOSDataSource()
{
    qDebug() << Q_FUNC_INFO;
}

QList<DataSource::SleepSession> ZeppOSDataSource::calculateSleep(const QDate &day)
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

    QString qry = "SELECT sleep_session_timestamp, sleep_session_raw_data FROM sleep_session WHERE sleep_session_timestamp >= " +
            QString::number(startDate.toMSecsSinceEpoch() / 1000) + " AND sleep_session_timestamp <= '" +
            QString::number(endDate.toMSecsSinceEpoch() / 1000) +   "' ORDER BY sleep_session_timestamp ASC";

    qDebug() << qry;

    if (m_conn && m_conn->isDatabaseUsed()) {
        KDbCursor *curs = m_conn->executeQuery(KDbEscapedString(qry));

        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {
                    int offset = 0;

                    qint64 d = curs->value(0).toLongLong();
                    QByteArray raw = QByteArray::fromBase64(curs->value(1).toString().toLocal8Bit());

                    qDebug() << "Raw sleep data length:" << raw.length();

                    if (raw.length() == 594) {
                        int32_t timestampSeconds = TypeConversion::toUint32(raw, offset);
                        QDateTime timestamp;
                        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);

                        int32_t timestampSecondsMidnight = TypeConversion::toUint32(raw, offset);
                        QDateTime timestampMidnight;
                        timestampMidnight.setMSecsSinceEpoch((qint64)timestampSecondsMidnight * 1000);

                        //skip 2 bytes
                        offset++;
                        offset++;

                        uint16_t sleepStart = TypeConversion::toUint16(raw, offset);
                        uint16_t sleepEnd = TypeConversion::toUint16(raw, offset);

                        offset = 0x15;
                        uint8_t avgHr = TypeConversion::toUint8(raw, offset);
                        uint8_t score = TypeConversion::toUint8(raw, offset);

                        offset = 0x54;

                        int numStages = TypeConversion::toUint8(raw, offset);

                        qDebug() << "Num sleep stages:" << numStages;
                        for (int i = 0; i < numStages; i++) {
                            offset = 0x56 + 5 * i;
                            uint16_t stageStart = TypeConversion::toUint16(raw, offset);
                            uint16_t stageEnd = TypeConversion::toUint16(raw, offset);
                            uint16_t stageType = TypeConversion::toUint8(raw, offset);

                            qDebug() << "Sleep stage:" << stageStart << stageEnd << stageType;
                        }

                        offset = 0x024a;
                        uint16_t totalRemMinutes = TypeConversion::toUint16(raw, offset);
                        uint16_t totalLightMinutes = TypeConversion::toUint16(raw, offset);
                        uint16_t totalDeepMinutes = TypeConversion::toUint16(raw, offset);
                        uint16_t totalWakeMinutes = TypeConversion::toUint16(raw, offset);

                        SleepSession ss;
                        ss.sleepStart = timestamp;
                        ss.sleepEnd = timestamp;
                        ss.lightSleepDuration = totalLightMinutes * 60;
                        ss.deepSleepDuration = totalDeepMinutes * 60;
                        ss.remSleepDuration = totalRemMinutes * 60;
                        ss.awakeDuration = totalWakeMinutes * 60;
                        sessions << ss;

                        qDebug() << "Sleep Session:" << ss.sleepStart << ss.lightSleepDuration << ss.deepSleepDuration << ss.remSleepDuration << ss.awakeDuration;
                    }
                    curs->moveNext();
                }
            }
            m_conn->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    }
    return sessions;
}
