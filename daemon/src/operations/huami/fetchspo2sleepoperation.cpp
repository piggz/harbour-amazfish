#include "fetchspo2sleepoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

FetchSpo2SleepOperation::FetchSpo2SleepOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastSpo2SleepTimeMillis");
}

void FetchSpo2SleepOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPO2_SLEEP) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchSpo2SleepOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 65);

    if ((m_buffer.length() - 1) % 30 != 0) {
        qDebug() << "Unexpected length for SPO2 sleep data, not divisible by 30" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<Spo2SleepRecord> recs;

    int version = m_buffer[0];
    if (version != 2) {
        qDebug() << "Unknown sleep spo2 data version " << version;
        return false;
    }

    for (int i = 1; i < m_buffer.length(); i+=30) {
        int offset = i;
        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);

        Spo2SleepRecord spo;
        spo.timestamp = timestamp;
        spo.value =  (uint8_t)m_buffer[offset++];
        spo.duration =  (uint8_t)m_buffer[offset++];
        spo.spo2High = m_buffer.mid(offset, 6);
        offset += 6;
        spo.spo2Low = m_buffer.mid(offset, 6);
        offset += 6;
        spo.spo2Quality = m_buffer.mid(offset, 8);
        offset += 8;

        recs << spo;
    }

    return saveRecords(recs);
}

bool FetchSpo2SleepOperation::saveRecords(QVector<Spo2SleepRecord> recs)
{
    bool success = true;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QDateTime lastTime;
    foreach(const auto &r, recs) {
        qDebug() << "Processing record:" << r.timestamp << r.value;

        if (db.isOpen()) {
            QString sql = QString("SELECT spo2sleep_id FROM spo2sleep WHERE spo2_timestamp=%1")
                              .arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            QSqlQuery query;
            bool exists = query.exec(sql) && query.next();
            qDebug() << sql << exists;

            lastTime = r.timestamp;

            if (!exists) {
                qDebug() << "SPO2 record does not exist, inserting";
                query.prepare("INSERT INTO spo2sleep (spo2sleep_timestamp, spo2sleep_timestamp_dt, spo2sleep_value, "
                              "spo2sleep_duration, spo2sleep_high, spo2sleep_low, spo2sleep_quality) "
                              "VALUES (?, ?, ?, ?, ?, ?, ?)");
                query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
                query.addBindValue(Database::dateTimeToText(r.timestamp));
                query.addBindValue(r.value);
                query.addBindValue(r.duration);
                query.addBindValue(r.spo2High);
                query.addBindValue(r.spo2Low);
                query.addBindValue(r.spo2Quality);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << "Error inserting spo2 record" << query.lastError().text();
                }
            } else {
                qDebug() << "SPO2 Sleep record exists, skipping";
            }
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
