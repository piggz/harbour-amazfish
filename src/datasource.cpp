#include "datasource.h"

DataSource::DataSource()
{
}

void DataSource::setConnection(KDbConnection *conn)
{
    m_conn = conn;
}

QVariant DataSource::data(Type type, const QDate &day)
{
    if (m_conn && m_conn->isDatabaseUsed()) {

    }
    return QVariant();
}
