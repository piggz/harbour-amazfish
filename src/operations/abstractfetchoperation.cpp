#include "abstractfetchoperation.h"
#include "mibandservice.h"
#include "typeconversion.h"

AbstractFetchOperation::AbstractFetchOperation(QBLEService *service) : AbstractOperation(service)
{

}

QDateTime AbstractFetchOperation::lastActivitySync()
{
    qlonglong ls = m_settings.value(m_lastSyncKey).toLongLong();

    if (ls == 0) {
        return QDateTime::currentDateTime().addDays(-100);
    }
    QTimeZone tz = QTimeZone(QTimeZone::systemTimeZone().standardTimeOffset(QDateTime::currentDateTime())); //Getting the timezone without DST

    qDebug() << "last sync was " << ls << QDateTime::fromMSecsSinceEpoch(ls, tz);
    return QDateTime::fromMSecsSinceEpoch(ls, tz);
}

void AbstractFetchOperation::saveLastActivitySync(qint64 millis)
{
    m_settings.setValue(m_lastSyncKey, millis);
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
        qDebug() << "Abort signalled from operation";
        return true;
    }
    if (value.length() == 15) {
        // first two bytes are whether our request was accepted
        if (value.mid(0, 3) == QByteArray(MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS, 3)) {
            // the third byte (0x01 on success) = ?
            // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
            int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

            // last 8 bytes are the start date
            QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), false);
            setStartDate(startDate);

            qDebug() << "About to transfer data from " << startDate;
            m_service->message(QObject::tr("About to transfer data from ") + startDate.toString());

        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else if (value.length() == 3) {
        if (value == QByteArray(MiBandService::RESPONSE_FINISH_SUCCESS, 3)) {
            qDebug() << "Finished sending data";
            finished(true);
            m_service->message(QObject::tr("Finished transferring activity data"));
            return true;
        } else if (value == QByteArray(MiBandService::RESPONSE_FINISH_FAIL, 3)) {
            qDebug() << "No data left to fetch";
            m_service->message(QObject::tr("No data to transfer"));
            return true;
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
