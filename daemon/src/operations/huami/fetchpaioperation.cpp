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
    qDebug() << Q_FUNC_INFO;
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

    for (int i = 0; i < m_buffer.length(); i+=102) {

        int offset = i;

        int type = m_buffer[offset++] & 0xff;

        if (type != 5 && type != 0) {
            qDebug() << "Unsupported PAI type "  << type;
            return false;
        }

        int32_t timestampSeconds = TypeConversion::toUint32(m_buffer, offset);
        timestamp.setMSecsSinceEpoch(timestampSeconds * 1000);
        uint8_t utcOffsetInQuarterHours = m_buffer[offset++];

        offset += 31; // unknown 31 bytes

        float paiLow = TypeConversion::toFloat(m_buffer, offset);
        float paiModerate = TypeConversion::toFloat(m_buffer, offset);
        float paiHigh = TypeConversion::toFloat(m_buffer, offset);
        short timeLow = TypeConversion::toUint16(m_buffer, offset); // minutes
        short timeModerate = TypeConversion::toUint16(m_buffer, offset); // minutes
        short timeHigh = TypeConversion::toUint16(m_buffer, offset); // minutes
        float paiToday = TypeConversion::toFloat(m_buffer, offset);
        float paiTotal = TypeConversion::toFloat(m_buffer, offset);

        //39 Bytes unknown
        qDebug() << "PAI" << timestamp.toString(Qt::ISODate) << paiLow << paiModerate << paiHigh << timeLow << timeModerate << timeHigh << paiToday << paiTotal;

        if (type == 0) {
            // Values from before the factory reset?
            qDebug() << "Ignoring PAI type 0";
        }
    }
    return true;
}
