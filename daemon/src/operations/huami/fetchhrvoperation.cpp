#include "fetchhrvoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"


FetchHrvOperation::FetchHrvOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastHrvTimeMillis");
}

void FetchHrvOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_HRV) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchHrvOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 102);

    if (m_buffer.length() % 6 != 0) {
        qDebug() << "Unexpected length for HRV data {}, not divisible by 6" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<HrvRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=6) {
        int offset = i;

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        uint8_t utcOffsetInQuarterHours = m_buffer[offset++];
        uint8_t val = m_buffer[offset] & 0xff;

        HrvRecord hrv;
        hrv.timestamp = timestamp;
        hrv.value = val;

        recs << hrv;
    }

    return saveRecords(recs);
}

bool FetchHrvOperation::saveRecords(QVector<HrvRecord> recs)
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
            KDbEscapedString sql = KDbEscapedString("SELECT hrv_id FROM hrv WHERE hrv_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            if (success == cancelled || success == false) {
                qDebug() << "HRV record does not exist, inserting";
                auto hrvData = m_conn->tableSchema("hrv");
                KDbFieldList hrvFields;
                hrvFields.addField(hrvData->field("hrv_timestamp"));
                hrvFields.addField(hrvData->field("hrv_timestamp_dt"));
                hrvFields.addField(hrvData->field("hrv_value"));

                QList<QVariant> hrvValues;
                hrvValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                hrvValues << r.timestamp;
                hrvValues << r.value;

                lastTime = r.timestamp;

                result = m_conn->insertRecord(&hrvFields, hrvValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting hrv record";
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
