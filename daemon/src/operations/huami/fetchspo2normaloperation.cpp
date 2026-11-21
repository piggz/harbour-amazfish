#include "fetchspo2normaloperation.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"
#include "mibandservice.h"

#include "database.h"

FetchSpo2NormalOperation::FetchSpo2NormalOperation(HuamiFetcher* fetcher, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
{
    setLastSyncKey("device/lastSpo2NormalTimeMillis");
}

void FetchSpo2NormalOperation::start()
{
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

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
    QVector<Database::Spo2Record> recs;

    int version = m_buffer[0];
    if (version != 2) {
        qDebug() << "Unknown normal spo2 data version " << version;
        return false;
    }

    for (int i = 1; i < m_buffer.length(); i+=65) {
        int offset = i;
        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch((qint64)timestampSeconds * 1000);
        int8_t spoRaw = (int8_t)m_buffer[offset++];
        bool automatic = spoRaw < 0;
        uint8_t spo2 = automatic ? (uint8_t)(spoRaw + 128) : (uint8_t)spoRaw;

        offset += 60; // unknown 60 bytes

        Database::Spo2Record spo;
        spo.timestamp = timestamp;
        spo.automatic = automatic;
        spo.value = spo2;

        recs << spo;
    }

    return saveRecords(recs);
}

bool FetchSpo2NormalOperation::saveRecords(QVector<Database::Spo2Record> recs)
{
    if (!Database::storeSpo2Records(recs)) {
        return false;
    }

    QDateTime lastTime;
    if (!recs.isEmpty()) {
        lastTime = recs.last().timestamp;
    }
    saveLastActivitySync(lastTime.toMSecsSinceEpoch());
    return true;
}
