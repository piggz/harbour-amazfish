#include "fetchsleepsessionoperation.h"
#include "huami/huamifetcher.h"
#include "typeconversion.h"
#include "mibandservice.h"

FetchSleepSessionOperation::FetchSleepSessionOperation(HuamiFetcher *fetcher, KDbConnection *conn) : AbstractFetchOperation(fetcher, true), m_conn(conn)
{
    setLastSyncKey("device/lastSleepSessionTimeMillis");
}

void FetchSleepSessionOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_SLEEP_SESSION) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchSleepSessionOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 594);

    if (m_buffer.length() % 594 != 0) {
        qDebug() << "Unexpected length for sleep session data {}, not divisible by 594" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<SleepSessionRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=594) {
        int offset = i * 594;

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);

        SleepSessionRecord slp;
        slp.timestamp = timestamp;
        slp.rawData = m_buffer.mid(i, 594).toBase64();

        recs << slp;
    }

    return saveRecords(recs);
}

bool FetchSleepSessionOperation::saveRecords(QVector<SleepSessionRecord> recs)
{
    QSharedPointer<KDbSqlResult> result;

    bool success = true;

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    QDateTime lastTime;
    foreach(const auto &r, recs) {
        qDebug() << "Processing record:" << r.timestamp;
        int count;

        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbEscapedString sql = KDbEscapedString("SELECT sleep_session_id FROM sleep_session WHERE sleep_session_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            lastTime = r.timestamp;

            if (success == cancelled || success == false) {
                qDebug() << "Sleep Session record does not exist, inserting";
                auto ssData = m_conn->tableSchema("sleep_session");
                KDbFieldList ssFields;
                ssFields.addField(ssData->field("sleep_session_timestamp"));
                ssFields.addField(ssData->field("sleep_session_timestamp_dt"));
                ssFields.addField(ssData->field("sleep_session_raw_value"));

                QList<QVariant> ssValues;
                ssValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                ssValues << r.timestamp;
                ssValues << r.rawData;

                result = m_conn->insertRecord(&ssFields, ssValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting sleep_session record";
                    success = false;
                }
            } else {
                qDebug() << "Sleep session record exists, skipping";
            }
        }
    }
    tg.commit();
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
