#include "fetchspo2sleepoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

FetchSpo2SleepOperation::FetchSpo2SleepOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastSpo2SleepTimeMillis");
}

void FetchSpo2SleepOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, true);

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
    QSharedPointer<KDbSqlResult> result;

    bool success = true;

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    QDateTime lastTime;
    foreach(const auto &r, recs) {
        qDebug() << "Processing record:" << r.timestamp << r.value;
        int count;

        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbEscapedString sql = KDbEscapedString("SELECT spo2sleep_id FROM spo2sleep WHERE spo2_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            lastTime = r.timestamp;

            if (success == cancelled || success == false) {
                qDebug() << "SPO2 record does not exist, inserting";
                auto spoData = m_conn->tableSchema("spo2sleep");
                KDbFieldList spoFields;
                spoFields.addField(spoData->field("spo2sleep_timestamp"));
                spoFields.addField(spoData->field("spo2sleep_timestamp_dt"));
                spoFields.addField(spoData->field("spo2sleep_value"));
                spoFields.addField(spoData->field("spo2sleep_duration"));
                spoFields.addField(spoData->field("spo2sleep_high"));
                spoFields.addField(spoData->field("spo2sleep_low"));
                spoFields.addField(spoData->field("spo2sleep_quality"));

                QList<QVariant> spoValues;
                spoValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                spoValues << r.timestamp;
                spoValues << r.value;
                spoValues << r.duration;
                spoValues << r.spo2High;
                spoValues << r.spo2Low;
                spoValues << r.spo2Quality;

                result = m_conn->insertRecord(&spoFields, spoValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting spo2 record";
                    success = false;
                }
            } else {
                qDebug() << "HRV record exists, skipping";
            }
        }
    }
    tg.commit();
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
