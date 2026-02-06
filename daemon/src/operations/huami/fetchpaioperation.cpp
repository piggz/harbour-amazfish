#include "fetchpaioperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

FetchPaiOperation::FetchPaiOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
{
    setLastSyncKey("device/lastPaiTimeMillis");
}

void FetchPaiOperation::start(QBLEService *service)
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_PAI) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchPaiOperation::processBufferedData()
{

    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 102);

    if (m_buffer.length() % 102 != 0) {
        qDebug() << "Unexpected length for PAI data {}, not divisible by 102" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<PaiRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=102) {

        int offset = i;

        int type = m_buffer[offset++] & 0xff;

        if (type != 5 && type != 0) {
            qDebug() << "Unsupported PAI type "  << type;
            return false;
        }

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        uint8_t utcOffsetInQuarterHours = m_buffer[offset++];

        offset += 31; // unknown 31 bytes

        PaiRecord pai;
        pai.day = timestamp.date();
        pai.low = TypeConversion::toFloat(m_buffer, offset);
        pai.moderate = TypeConversion::toFloat(m_buffer, offset);
        pai.high = TypeConversion::toFloat(m_buffer, offset);
        pai.time_low = TypeConversion::toUint16(m_buffer, offset); // minutes
        pai.time_moderate= TypeConversion::toUint16(m_buffer, offset); // minutes
        pai.time_high = TypeConversion::toUint16(m_buffer, offset); // minutes
        pai.total_today = TypeConversion::toFloat(m_buffer, offset);
        pai.total = TypeConversion::toFloat(m_buffer, offset);

        recs << pai;

        //39 Bytes unknown

        if (type == 0) {
            // Values from before the factory reset?
            qDebug() << "Ignoring PAI type 0";
        }
    }

    return saveRecords(recs);
}

bool FetchPaiOperation::saveRecords(QVector<PaiRecord> recs)
{
    QSharedPointer<KDbSqlResult> result;

    bool success = true;

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    foreach(const auto &r, recs) {
        int count;

        qDebug() << "Processing record:" << r.day << r.total_today << r.total;
        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbEscapedString sql = KDbEscapedString("SELECT id FROM pai WHERE pai_date='%1'").arg(r.day.toString(Qt::ISODate));
            tristate success = m_conn->querySingleNumber(sql, &count);
            qDebug() << sql << success << count;

            if (success == cancelled || success == false) {
                qDebug() << "PAI record does not exist, inserting";
                auto paiData = m_conn->tableSchema("pai");
                KDbFieldList paiFields;
                paiFields.addField(paiData->field("pai_date"));
                paiFields.addField(paiData->field("pai_low"));
                paiFields.addField(paiData->field("pai_moderate"));
                paiFields.addField(paiData->field("pai_high"));
                paiFields.addField(paiData->field("pai_low_time"));
                paiFields.addField(paiData->field("pai_moderate_time"));
                paiFields.addField(paiData->field("pai_high_time"));
                paiFields.addField(paiData->field("pai_today"));
                paiFields.addField(paiData->field("pai_total"));

                QList<QVariant> paiValues;
                paiValues << r.day.toString(Qt::ISODate);
                paiValues << r.low;
                paiValues << r.moderate;
                paiValues << r.high;
                paiValues << r.time_low;
                paiValues << r.time_moderate;
                paiValues << r.time_high;
                paiValues << r.total_today;
                paiValues << r.total;

                result = m_conn->insertRecord(&paiFields, paiValues);
                if (result->lastResult().isError()) {
                    qDebug() << Q_FUNC_INFO << "Error inserting meta record";
                    success = false;
                }
            } else {
                qDebug() << "PAI record exists, updating";
                if (!m_conn->executeSql(KDbEscapedString("UPDATE pai SET "
                                                         "pai_low=%1, "
                                                         "pai_moderate=%2, "
                                                         "pai_high=%3, "
                                                         "pai_low_time=%4, "
                                                         "pai_moderate_time=%5, "
                                                         "pai_high_time=%6, "
                                                         "pai_today=%7, "
                                                         "pai_total=%8 "
                                                         "WHERE pai_date='%9'")
                                        .arg(r.low)
                                        .arg(r.moderate)
                                        .arg(r.high)
                                        .arg(r.time_low)
                                        .arg(r.time_moderate)
                                        .arg(r.time_high)
                                        .arg(r.total_today)
                                        .arg(r.total)
                                        .arg(r.day.toString(Qt::ISODate))))

                {
                    qDebug() << Q_FUNC_INFO << "Error inserting pai record";
                    success = false;
                }
            }
        }
    }
    tg.commit();
    return success;
}

