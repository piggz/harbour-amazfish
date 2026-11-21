#include "fetchpaioperation.h"
#include "database.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"
#include "typeconversion.h"

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
    QVector<Database::PaiRecord> recs;

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

        Database::PaiRecord pai;
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

bool FetchPaiOperation::saveRecords(QVector<Database::PaiRecord> recs)
{
    QDate lastNewDay;
    if (!Database::storePaiRecords(recs, lastNewDay)) {
        return false;
    }

    QDateTime lastTime;
    if (lastNewDay.isValid()) {
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        lastTime = QDateTime(lastNewDay);
#else
        lastTime = lastNewDay.startOfDay();
#endif
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}

