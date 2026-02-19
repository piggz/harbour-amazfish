#include "fetchspo2normaloperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

FetchSpo2NormalOperation::FetchSpo2NormalOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastSpo2NormalTimeMillis");
}

void FetchSpo2NormalOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPO2_NORMAL) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchSpo2NormalOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 65);

    if ((m_buffer.length() - 1) % 65 != 0) {
        qDebug() << "Unexpected length for SPO2 data, not divisible by 65" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<Spo2Record> recs;

    int version = m_buffer[0];
    if (version != 2) {
        qDebug() << "Unknown normal spo2 data version " << version;
        return false;
    }

    for (int i = 1; i < m_buffer.length(); i+=65) {
        int offset = i;
        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        char spoRaw = m_buffer[offset++];
        bool automatic = spoRaw < 0;
        uint8_t spo2 = (uint8_t) (spoRaw < 0 ? spoRaw + 128 : spoRaw);

        offset += 60; // unknown 60 bytes

        Spo2Record spo;
        spo.timestamp = timestamp;
        spo.automatic = automatic;
        spo.value = spo2;

        recs << spo;
    }

    return saveRecords(recs);
}

bool FetchSpo2NormalOperation::saveRecords(QVector<Spo2Record> recs)
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
            KDbEscapedString sql = KDbEscapedString("SELECT spo2_id FROM spo2 WHERE spo2_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            lastTime = r.timestamp;

            if (success == cancelled || success == false) {
                qDebug() << "SPO2 record does not exist, inserting";
                auto spoData = m_conn->tableSchema("spo2");
                KDbFieldList spoFields;
                spoFields.addField(spoData->field("spo2_timestamp"));
                spoFields.addField(spoData->field("spo2_timestamp_dt"));
                spoFields.addField(spoData->field("spo2_automatic"));
                spoFields.addField(spoData->field("spo2_value"));

                QList<QVariant> spoValues;
                spoValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                spoValues << r.timestamp;
                spoValues << r.automatic;
                spoValues << r.value;

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
