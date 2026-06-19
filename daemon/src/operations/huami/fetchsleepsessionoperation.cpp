#include "fetchsleepsessionoperation.h"
#include "huami/huamifetcher.h"
#include "typeconversion.h"
#include "mibandservice.h"

FetchSleepSessionOperation::FetchSleepSessionOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_conn(conn)
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
    qDebug() << Q_FUNC_INFO << m_buffer.length() << (m_buffer.length() / 102);

    if (m_buffer.length() % 594 != 0) {
        qDebug() << "Unexpected length for HRV data {}, not divisible by 594" << m_buffer.length();
        return false;
    }

    QDateTime timestamp;
    QVector<SleepSessionRecord> recs;

    for (int i = 0; i < m_buffer.length(); i+=6) {
        int offset = i;

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        uint8_t utcOffsetInQuarterHours = m_buffer[offset++];
        uint8_t val = m_buffer[offset] & 0xff;

        SleepSessionRecord slp;


        recs << slp;
    }

    return saveRecords(recs);
}

bool FetchSleepSessionOperation::saveRecords(QVector<SleepSessionRecord> recs)
{

}
