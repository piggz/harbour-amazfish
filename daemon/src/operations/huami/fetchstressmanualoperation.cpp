#include "fetchstressmanualoperation.h"
#include "huami/huamifetcher.h"
#include "typeconversion.h"
#include "mibandservice.h"

#include "database.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

FetchStressManualOperation::FetchStressManualOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastStressManualTimeMillis");
}

void FetchStressManualOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_STRESS_MANUAL) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchStressManualOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << m_buffer.toHex();

    if ((m_buffer.length()) % 5 != 0) {
        qDebug() << "Unexpected length for manual stress data, not divisible by 5" << m_buffer.length();
        return false;
    }

    QDateTime timestamp = startDateLocal();
    QVector<StressRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=5) {
        // 0-39 = relaxed
        // 40-59 = mild
        // 60-79 = moderate
        // 80-100 = high

        int offset = i;

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);

        int8_t b = m_buffer[offset];

        if (b == -1) {
            timestamp = timestamp.addSecs(60);
            continue;
        }

        StressRecord rec;

        rec.timestamp = timestamp;
        rec.type = 2; //2 = Manual
        rec.value = b & 0xff;

        timestamp = timestamp.addSecs(60);
        recs << rec;
    }

    return saveRecords(recs, timestamp);
}

bool FetchStressManualOperation::saveRecords(QVector<StressRecord> recs, QDateTime lastTimeDefault)
{
    bool success = true;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QDateTime lastTime = lastTimeDefault;
    foreach(const auto &r, recs) {
        qDebug() << "Processing record:" << r.timestamp << r.value;

        if (db.isOpen()) {
            QString sql = QString("SELECT stress_id FROM stress WHERE stress_timestamp=%1 AND stress_type=2")
                              .arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            QSqlQuery query;
            bool exists = query.exec(sql) && query.next();
            qDebug() << sql << exists;

            lastTime = r.timestamp;

            if (!exists) {
                query.prepare("INSERT INTO stress (stress_timestamp, stress_timestamp_dt, stress_value, stress_type) "
                              "VALUES (?, ?, ?, ?)");
                query.addBindValue(r.timestamp.toMSecsSinceEpoch() / 1000);
                query.addBindValue(Database::dateTimeToText(r.timestamp));
                query.addBindValue(r.value);
                query.addBindValue(r.type);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << "Error inserting stress record" << query.lastError().text();
                }
            } else {
                qDebug() << "Stress record exists, skipping";
            }
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    lastTime = lastTime.addSecs(60);
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}