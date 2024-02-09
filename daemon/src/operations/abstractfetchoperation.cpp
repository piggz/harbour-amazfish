#include "abstractfetchoperation.h"
#include "mibandservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"

AbstractFetchOperation::AbstractFetchOperation(QBLEService *service) : AbstractOperation(service)
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
    if (m_abort) {
        qDebug() << Q_FUNC_INFO << ": Abort signalled from operation";
        return true;
    }
    qDebug() << Q_FUNC_INFO << value;
    if (value.length() == 15) {
        // first two bytes are whether our request was accepted
        if (value.mid(0, 3) == UCHARARR_TO_BYTEARRAY(MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS)) {
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

        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else if (value.length() == 3) {
        if (value == UCHARARR_TO_BYTEARRAY(MiBandService::RESPONSE_FINISH_SUCCESS)) {
            qDebug() << "Finished sending data";
            finished(true);
            m_service->message(QObject::tr("Finished transferring activity data"));
            return true;
        } else if (value == UCHARARR_TO_BYTEARRAY(MiBandService::RESPONSE_FINISH_FAIL)) {
            qDebug() << "No data left to fetch";
            m_service->message(QObject::tr("No data to transfer"));
            return true;
        } else if (value == UCHARARR_TO_BYTEARRAY(MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS)) {
            qDebug() << "Activity start date success";
            //Send log read command
            m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else {
        qDebug() << "Unexpected activity metadata: " << value;
    }
    return false;
}

void AbstractFetchOperation::setAbort(bool abort)
{
    m_abort = abort;
}
