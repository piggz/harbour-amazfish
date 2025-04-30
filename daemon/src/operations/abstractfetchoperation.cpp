#include "abstractfetchoperation.h"
#include "mibandservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"

AbstractFetchOperation::AbstractFetchOperation()
{

}

QDateTime AbstractFetchOperation::lastActivitySync()
{
    qlonglong ls = AmazfishConfig::instance()->value(m_lastSyncKey).toLongLong();

    if (ls == 0) {
        return QDateTime::currentDateTime().addDays(-100);
    }

    //QTimeZone tz = QTimeZone(QTimeZone::systemTimeZone().standardTimeOffset(QDateTime::currentDateTime())); //Getting the timezone without DST

    //Convert the last sync time, which is seconds since epoch, to a qdatetime in the local timezone
    qDebug() << Q_FUNC_INFO << ": Last sync was " << ls << QDateTime::fromMSecsSinceEpoch(ls, Qt::LocalTime);
    return QDateTime::fromMSecsSinceEpoch(ls, Qt::LocalTime);
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
    qDebug() << Q_FUNC_INFO;
    if (!m_service) {
        qDebug() << "No service set";
        return true;
    }

    if (m_abort) {
        qDebug() << Q_FUNC_INFO << ": Abort signalled from operation";
        return true;
    }
    qDebug() << Q_FUNC_INFO << value;

    if (value.length() < 3) {
        qDebug() << "Activity metadata too short";
        finished(false);
        return true;
    }

    if (value[0] != MiBandService::RESPONSE) {
        qDebug() << "Activity metadata not a response: " << value[0];
        finished(false);
        return true;
    }


    switch (value[1]) {
    case MiBandService::COMMAND_ACTIVITY_DATA_START_DATE:
        return handleStartDateResponse(value);
    case MiBandService::COMMAND_FETCH_DATA:
        handleFetchDataResponse(value);
        return false;
    case MiBandService::COMMAND_ACK_ACTIVITY_DATA:
        qDebug() << "Got reply to COMMAND_ACK_ACTIVITY_DATA";
        finished(true);
        return true;
    default:
        qDebug() << "Unexpected activity metadata: " << value;
        finished(true);
        return true;
    }

    return false;
}

bool AbstractFetchOperation::handleStartDateResponse(const QByteArray &value)
{
    if (value[2] != MiBandService::SUCCESS) {
        finished(false);
        return true;
    }

    // it's 16 on the MB7, with a 0 at the end
    if (value.length() != 15 && (value.length() != 16 && value[15] != 0x00)) {
        qDebug() << "Start date response length: " << value.length();
        finished(false);
        return true;
    }

    // the third byte (0x01 on success) = ?
    // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
    int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

    // last 8 bytes are the start date
    //! Here, we read the start date/time and dont apply the TZ offset.  We then apply the local
    //! TZ.  This should work in most cases i think!
    QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), false);
    startDate.setTimeZone(QTimeZone::systemTimeZone());
    setStartDate(startDate);

    qDebug() << "About to transfer data from " << startDate << "expected length:" << expectedDataLength; //TODO use expectedDataLength
    m_service->message(QObject::tr("About to transfer data from ") + startDate.toString());
    //Send log read command
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));

    return false;

}

bool AbstractFetchOperation::handleFetchDataResponse(const QByteArray &value)
{
    if (value.length() != 3 && value.length() != 7) {
        qDebug() << "Fetch data unexpected metadata length: " << value.length();
        finished(false);
        return true;
    }

    if (value[2] != MiBandService::SUCCESS) {
        m_service->message(QObject::tr("No data to transfer"));
        finished(true);
        return true;
    }

    //TODO handle size 7
    finished(true);
    return true;

}

void AbstractFetchOperation::setAbort(bool abort)
{
    m_abort = abort;
}
