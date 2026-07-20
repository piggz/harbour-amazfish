#include "zepposdatasource.h"


ZeppOSDataSource::ZeppOSDataSource()
{
    qDebug() << Q_FUNC_INFO;
}

QList<DataSource::SleepSession> ZeppOSDataSource::calculateSleep(const QDate &day)
{
    return QList<DataSource::SleepSession>();
}
