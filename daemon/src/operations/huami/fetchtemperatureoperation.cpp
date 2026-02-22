#include "fetchtemperatureoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

FetchTemperatureOperation::FetchTemperatureOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastTemperatureTimeMillis");
}

void FetchTemperatureOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_TEMPERATURE) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchTemperatureOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 65);

    if (m_buffer.length() % 8 != 0) {
        qDebug() << "Unexpected length for SPO2 sleep data, not divisible by 8" << m_buffer.length();
        return false;
    }

    QDateTime timestamp = startDate();
    QVector<TemperatureRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=8) {
        int offset = i;

        uint16_t unk1 = TypeConversion::toUint16(m_buffer, offset);
        uint16_t t = TypeConversion::toUint16(m_buffer, offset);
        uint16_t unk2 = TypeConversion::toUint16(m_buffer, offset);
        uint16_t unk3 = TypeConversion::toUint16(m_buffer, offset);


        TemperatureRecord temp;
        temp.timestamp = timestamp;
        temp.value = t / 100.0f;
        temp.location = "wrist";
        temp.type = "skin";

        recs << temp;
        timestamp = timestamp.addSecs(60);
    }

    return saveRecords(recs);
}

bool FetchTemperatureOperation::saveRecords(QVector<TemperatureRecord> recs)
{
    QSharedPointer<KDbSqlResult> result;

    bool success = true;

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    QDateTime lastTime;
    foreach(const auto &r, recs) {
        int count;

        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbEscapedString sql = KDbEscapedString("SELECT temperature_id FROM temperature WHERE temperature_timestamp=%1").arg(r.timestamp.toMSecsSinceEpoch() / 1000);
            tristate success = m_conn->querySingleNumber(sql, &count);

            lastTime = r.timestamp;

            if (success == cancelled || success == false) {
                auto tempData = m_conn->tableSchema("temperature");
                KDbFieldList tempFields;
                tempFields.addField(tempData->field("temperature_timestamp"));
                tempFields.addField(tempData->field("temperature_timestamp_dt"));
                tempFields.addField(tempData->field("temperature_value"));
                tempFields.addField(tempData->field("temperature_location"));
                tempFields.addField(tempData->field("temperature_type"));

                QList<QVariant> tempValues;
                tempValues << r.timestamp.toMSecsSinceEpoch() / 1000;
                tempValues << r.timestamp;
                tempValues << r.value;
                tempValues << r.location;
                tempValues << r.type;

                result = m_conn->insertRecord(&tempFields, tempValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting temperature record";
                    success = false;
                }
            } else {
                qDebug() << "Temp record exists, skipping";
            }
        }
    }
    tg.commit();
    lastTime = lastTime.addSecs(60);
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
