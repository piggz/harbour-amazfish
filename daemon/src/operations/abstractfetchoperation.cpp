#include "abstractfetchoperation.h"
#include "mibandservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"
#include "huami/huamifetcher.h"

AbstractFetchOperation::AbstractFetchOperation(HuamiFetcher *fetcher, bool isZeppOs) : m_fetcher(fetcher), m_isZeppOs(isZeppOs)
{

}

QDateTime AbstractFetchOperation::lastActivitySync()
{
    qlonglong ls = AmazfishConfig::instance()->value(m_lastSyncKey).toLongLong();

    if (ls <= 0) {
        return QDateTime::currentDateTime().addDays(-100);
    }

    //QTimeZone tz = QTimeZone(QTimeZone::systemTimeZone().standardTimeOffset(QDateTime::currentDateTime())); //Getting the timezone without DST

    //Convert the last sync time, which is seconds since epoch, to a qdatetime in the local timezone
    qDebug() << Q_FUNC_INFO << ": Last sync was " << ls << QDateTime::fromMSecsSinceEpoch(ls, QTimeZone::utc());
    return QDateTime::fromMSecsSinceEpoch(ls, QTimeZone::utc());
}

void AbstractFetchOperation::saveLastActivitySync(qint64 millis)
{
    AmazfishConfig::instance()->setValue(m_lastSyncKey, millis);
}

void AbstractFetchOperation::setStartDate(const QDateTime &sd)
{
    m_startDate = sd;
}

QDateTime AbstractFetchOperation::startDate() const
{
    return m_startDate;
}

void AbstractFetchOperation::setLastSyncKey(const QString &key)
{
    m_lastSyncKey = key;
}

bool AbstractFetchOperation::handleMetaData(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << value;

    if (m_abort) {
        qDebug() << Q_FUNC_INFO << ": Abort signalled from operation";
        return true;
    }

    if (value.length() < 3) {
        qDebug() << "Activity metadata too short";
        return false;
    }

    if (value[0] != MiBandService::RESPONSE) {
        qDebug() << "Activity metadata not a response: " << value[0];
        return false;
    }


    switch (value[1]) {
    case MiBandService::COMMAND_ACTIVITY_DATA_START_DATE:
        return !handleStartDateResponse(value);
    case MiBandService::COMMAND_FETCH_DATA:
        return handleFetchDataResponse(value);
    case MiBandService::COMMAND_ACK_ACTIVITY_DATA:
        qDebug() << "Got reply to COMMAND_ACK_ACTIVITY_DATA";
        return true;
    default:
        qDebug() << "Unexpected activity metadata: " << value;
        return true;
    }

    return false;
}

bool AbstractFetchOperation::handleStartDateResponse(const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;

    if (value[2] != MiBandService::SUCCESS) {
        return false;
    }

    // it's 16 on the MB7, with a 0 at the end
    if (value.length() != 15 && (value.length() != 16 && value[15] != 0x00)) {
        qDebug() << "Start date response length: " << value.length();
        return false;
    }

    // the third byte (0x01 on success) = ?
    // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
    int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

    // last 8 bytes are the start date
    //! Here, we read the start date/time and dont apply the TZ offset.  We then apply the local
    //! TZ.  This should work in most cases i think!
    QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), true);
    QDateTime local = startDate.toLocalTime();
    qDebug() << "DEBG:" << startDate << startDate.toMSecsSinceEpoch() << local;

    setStartDate(local);

    qDebug() << "About to transfer data from " << startDate << "expected length:" << expectedDataLength; //TODO use expectedDataLength
    m_fetcher->message(QObject::tr("About to transfer data from ") + startDate.toString());
    //Send read command
    m_fetcher->writeControl(QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
    return true;

}

bool AbstractFetchOperation::handleFetchDataResponse(const QByteArray &value)
{
    if (value.length() != 3 && value.length() != 7) {
        qDebug() << "Fetch data unexpected metadata length: " << value.length();
        return false;;
    }

    if (value[2] != MiBandService::SUCCESS) {
        m_fetcher->message(QObject::tr("No data to transfer"));
        return true;
    }

    //TODO handle size 7

    bool success = m_valid && processBufferedData();
    if (success && m_isZeppOs) {
        qDebug() << "Sending Ack";
        sendAck();
        return false;
    }

    return true;
}

void AbstractFetchOperation::sendAck()
{
    qDebug() << Q_FUNC_INFO;

    if (m_isZeppOs) {
        QByteArray cmd;

        bool keepDataOnDevice = true;

        cmd += MiBandService::COMMAND_ACK_ACTIVITY_DATA;
        cmd += (keepDataOnDevice ? 0x09 : 0x01);
        m_fetcher->writeControl(cmd);
    }
}

void AbstractFetchOperation::setAbort(bool abort)
{
    m_abort = abort;
}
