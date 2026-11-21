#include "fetchstressautooperation.h"
#include "huami/huamifetcher.h"
#include "typeconversion.h"
#include "mibandservice.h"

#include "database.h"

FetchStressAutoOperation::FetchStressAutoOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastStressAutoTimeMillis");
}

void FetchStressAutoOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_STRESS_AUTO) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool FetchStressAutoOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << m_buffer.length() << m_buffer.toHex();

    QDateTime timestamp = startDateLocal();
    QVector<Database::StressRecord> recs;

    for (int i = 0; i < m_buffer.length(); i++) {
        // 0-39 = relaxed
        // 40-59 = mild
        // 60-79 = moderate
        // 80-100 = high

        int8_t b = m_buffer[i];

        if (b == -1) {
            timestamp = timestamp.addSecs(60);
            continue;
        }

        Database::StressRecord rec;

        rec.timestamp = timestamp;
        rec.type = 1;
        rec.value = b & 0xff;

        timestamp = timestamp.addSecs(60);
        recs << rec;
    }

    return saveRecords(recs, timestamp);
}

bool FetchStressAutoOperation::saveRecords(QVector<Database::StressRecord> recs, QDateTime lastTimeDefault)
{
    if (!Database::storeStressRecords(recs)) {
        return false;
    }

    QDateTime lastTime = lastTimeDefault;
    if (!recs.isEmpty()) {
        lastTime = recs.last().timestamp;
    }
    lastTime = lastTime.addSecs(60);
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}