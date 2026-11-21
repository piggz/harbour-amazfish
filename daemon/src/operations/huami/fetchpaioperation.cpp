#include "fetchpaioperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

FetchPaiOperation::FetchPaiOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastPaiTimeMillis");
}

void FetchPaiOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

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
    bool success = true;

    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QDateTime lastTime;
    foreach(const auto &r, recs) {
        qDebug() << "Processing record:" << r.day << r.total_today << r.total;
        if (db.isOpen()) {
            QString sql = QString("SELECT id FROM pai WHERE pai_date='%1'").arg(r.day.toString(Qt::ISODate));
            QSqlQuery query;
            bool exists = query.exec(sql) && query.next();
            qDebug() << sql << exists;

            if (!exists) {
                qDebug() << "PAI record does not exist, inserting";
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
                lastTime = QDateTime(r.day);
#else
                lastTime = r.day.startOfDay();
#endif
                query.prepare("INSERT INTO pai (pai_date, pai_low, pai_moderate, pai_high, pai_low_time, "
                              "pai_moderate_time, pai_high_time, pai_today, pai_total) "
                              "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
                query.addBindValue(r.day.toString(Qt::ISODate));
                query.addBindValue(r.low);
                query.addBindValue(r.moderate);
                query.addBindValue(r.high);
                query.addBindValue(r.time_low);
                query.addBindValue(r.time_moderate);
                query.addBindValue(r.time_high);
                query.addBindValue(r.total_today);
                query.addBindValue(r.total);
                if (!query.exec()) {
                    qWarning() << Q_FUNC_INFO << "Error inserting meta record" << query.lastError().text();
                }
            } else {
                qDebug() << "PAI record exists, updating";
                if (!query.exec(QString("UPDATE pai SET "
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
                    qWarning() << Q_FUNC_INFO << "Error updating pai record" << query.lastError().text();
                    success = false;
                }
            }
        }
    }
    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save records:" << db.lastError().text();
        return false;
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return success;
}

