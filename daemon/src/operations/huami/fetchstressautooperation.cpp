#include "fetchstressautooperation.h"
#include "huami/huamifetcher.h"
#include "typeconversion.h"
#include "mibandservice.h"


FetchStressAutoOperation::FetchStressAutoOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastStressAutoTimeMillis");
}

void FetchStressAutoOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPO2_SLEEP) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchStressAutoOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 65);

    QDateTime timestamp = startDateLocal();
    QVector<StressRecord> recs;


    for (int i = 0; i < m_buffer.length(); i++) {
        // 0-39 = relaxed
        // 40-59 = mild
        // 60-79 = moderate
        // 80-100 = high

        int b = m_buffer[i] & 0xff;
        if (b == -1) {
            timestamp = timestamp.addSecs(60);
            continue;
        }

        StressRecord rec;

        rec.timestamp = timestamp;
        rec.type = 1;
        rec.value = b;

        timestamp = timestamp.addSecs(60);
        recs << rec;
    }

    return saveRecords(recs);
}

bool FetchStressAutoOperation::saveRecords(QVector<StressRecord> recs)
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
            KDbEscapedString sql = KDbEscapedString("SELECT stress_id FROM stress WHERE stress_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            lastTime = r.timestamp;

            if (success == cancelled || success == false) {
                auto spoData = m_conn->tableSchema("stress");
                KDbFieldList stressFields;
                stressFields.addField(spoData->field("stress_timestamp"));
                stressFields.addField(spoData->field("stress_timestamp_dt"));
                stressFields.addField(spoData->field("stress_value"));
                stressFields.addField(spoData->field("stress_type"));

                QList<QVariant> stressValues;
                stressValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                stressValues << r.timestamp;
                stressValues << r.value;
                stressValues << r.type;

                result = m_conn->insertRecord(&stressFields, stressValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting stress record";
                    success = false;
                }
            } else {
                qDebug() << "Stress record exists, skipping";
            }
        }
    }
    tg.commit();
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}