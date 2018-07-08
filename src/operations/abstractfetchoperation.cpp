#include "abstractfetchoperation.h"

AbstractFetchOperation::AbstractFetchOperation(QBLEService *service) : AbstractOperation(service)
{

}

QDateTime AbstractFetchOperation::lastActivitySync()
{
    qlonglong ls = m_settings.value(m_lastSyncKey).toLongLong();

    if (ls == 0) {
        return QDateTime::currentDateTime().addDays(-30);
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
