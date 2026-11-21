#include "fetchhrvoperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

#include "database.h"

FetchHrvOperation::FetchHrvOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastHrvTimeMillis");
}

void FetchHrvOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

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
    QVector<Database::HrvRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=6) {
        int offset = i;

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        uint8_t utcOffsetInQuarterHours = m_buffer[offset++];
        uint8_t val = m_buffer[offset] & 0xff;

        Database::HrvRecord hrv;
        hrv.timestamp = timestamp;
        hrv.value = val;

        recs << hrv;
    }

    return saveRecords(recs);
}

bool FetchHrvOperation::saveRecords(QVector<Database::HrvRecord> recs)
{
    if (!Database::storeHrvRecords(recs)) {
        return false;
    }

    QDateTime lastTime;
    if (!recs.isEmpty()) {
        lastTime = recs.last().timestamp;
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
