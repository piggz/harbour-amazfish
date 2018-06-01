#include "datasource.h"

DataSource::DataSource(SystemSnapshot *parent):
    QObject(parent)
{
    m_snapshot = parent;
}

int DataSource::registerSystemSource(const QString &source)
{
    return m_snapshot->registerSystemSource(source);
}

int DataSource::registerApplicationSource(const QString &source)
{
    return m_snapshot->registerApplicationSource(source);
}

const QByteArray & DataSource::getSystemData(int source)
{
    return m_snapshot->getSystemData(source);
}
