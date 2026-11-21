#include "fetchspo2sleepoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

#include "database.h"

FetchSpo2SleepOperation::FetchSpo2SleepOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastSpo2SleepTimeMillis");
}

void FetchSpo2SleepOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

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
    QVector<Database::Spo2SleepRecord> recs;

    int version = m_buffer[0];
    if (version != 2) {
        qDebug() << "Unknown sleep spo2 data version " << version;
        return false;
    }

    for (int i = 1; i < m_buffer.length(); i+=30) {
        int offset = i;
        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);

        Database::Spo2SleepRecord spo;
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

bool FetchSpo2SleepOperation::saveRecords(QVector<Database::Spo2SleepRecord> recs)
{
    if (!Database::storeSpo2SleepRecords(recs)) {
        return false;
    }

    QDateTime lastTime;
    if (!recs.isEmpty()) {
        lastTime = recs.last().timestamp;
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
