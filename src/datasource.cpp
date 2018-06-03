#include "datasource.h"
#include <QVariant>

DataSource::DataSource()
{
}

void DataSource::setConnection(KDbConnection *conn)
{
    m_conn = conn;
}

QVariant DataSource::data(Type type, const QDate &day)
{
    QList<QVariant> result;

    QString qry = "SELECT timestamp_dt, heartrate FROM mi_band_activity WHERE heartrate < 255 AND timestamp_dt >= '" +
            day.toString("yyyy-MM-ddT00:00:00") +  "' AND timestamp_dt <= '" +
            day.toString("yyyy-MM-ddT23:59:59") +  "' ORDER BY timestamp_dt ASC";

    qDebug() << qry;
    if (m_conn && m_conn->isDatabaseUsed()) {
        KDbCursor *curs = m_conn->executeQuery(KDbEscapedString(qry));

        if (curs) {
            if (curs->open() && curs->moveFirst()) {
                while (!curs->eof()) {

                    QVariant d = curs->value(0);
                    QVariant h = curs->value(1);

                    QVariantMap pt;
                    pt["x"] = d.toDateTime().toMSecsSinceEpoch() / 1000;
                    pt["y"] = h.toInt();
                    result.append(pt);

                    qDebug() << "Got data" << d << h;
                    curs->moveNext();
                }
            }
            m_conn->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    }
    return result;
}
