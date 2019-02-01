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

    QString qry;

    if (type == DataSource::SleepSummary) {
        QDate day2(2018, 6, 6);
        qry = "SELECT timestamp_dt, raw_kind, raw_intensity FROM mi_band_activity WHERE timestamp_dt >= date('" +
                day.toString("yyyy-MM-ddT11:59:00") + "','-10 day') AND timestamp_dt <= '" +
                day.toString("yyyy-MM-ddT12:01:00") +  "' ORDER BY timestamp_dt ASC";

        qDebug() << qry;
        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbCursor *curs = m_conn->executeQuery(KDbEscapedString(qry));

            if (curs) {
                if (curs->open() && curs->moveFirst()) {
                    QDate d; //Current day, sleepd detection runs from midday to midday
                    int light = 0; //minutes in light sleep
                    int deep = 0; //mintes in deep sleep
                    QDate curDate = curs->value(0).toDateTime().date();

                    QVariantMap pt;
                    int temp_sleep = 0;
                    bool in_sleep = false;

                    while (!curs->eof()) {

                        QVariant t = curs->value(0);
                        QVariant k = curs->value(1);
                        QVariant i = curs->value(2);

                        curDate = t.toDateTime().date();

                        if (curDate != d && t.toDateTime().time() > QTime(12, 00)) { //date change

                            //qDebug() << "Date changed:" << curDate << light << deep;
                            //Save current values
                            pt["x"] = QDateTime(d).toMSecsSinceEpoch() / 1000;
                            pt["y"] = light / 60.0;
                            pt["z"] = deep / 60.0;
                            result.append(pt);

                            //Reset values
                            d = curDate;
                            light = 0;
                            deep = 0;
                            in_sleep = false;
                        }

                        int activity = k.toInt();
                        int intensity = i.toInt();

                        if (activity == 123 || (in_sleep == false && activity)) {
                            //qDebug() << "Starting sleep tracking";
                            in_sleep = true;
                        }

                        if (activity == 124 || (in_sleep && ((activity & 112) != 112))) {
                            //qDebug() << "Stopping sleep tracking";

                            in_sleep = false;
                            //qDebug() << "Adding " << temp_sleep << " minutes of sleep";

                            if (temp_sleep > 10) { //add to deep
                                deep += temp_sleep;
                            } else {
                                light += temp_sleep;
                            }
                            temp_sleep = 0;
                        }

                        if (in_sleep && (activity == 112 && activity != 115 || activity == 121 || activity == 122)) {
                            //qDebug() << "Data:" << t << k << i;

                            if (intensity > 0) { //Not deep so just add to light
                                //qDebug() << "Adding " << temp_sleep << " minutes of sleep";

                                if (temp_sleep > 10) { //add to deep
                                    deep += temp_sleep;
                                } else {
                                    light += temp_sleep;
                                }
                                temp_sleep = 0;
                                light++;
                            } else {
                                temp_sleep++;
                            }
                        }
                        curs->moveNext();
                    }
                }
                //qDebug() << result;
            }
            m_conn->deleteCursor(curs);
        } else {
            qDebug() << "Error executing query";
        }
    } else {
        if (type == DataSource::Heartrate) {
            qry = "SELECT timestamp_dt, heartrate FROM mi_band_activity WHERE heartrate < 255 AND timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") +  "' AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' ORDER BY timestamp_dt ASC";
        } else if (type == DataSource::Steps) {
            qry = "SELECT timestamp_dt, steps FROM mi_band_activity WHERE timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") +  "' AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' ORDER BY timestamp_dt ASC";
        } else if (type == DataSource::Intensity) {
            qry = "SELECT timestamp_dt, ((raw_intensity / 255.0) * 100) FROM mi_band_activity WHERE timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") +  "' AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' ORDER BY timestamp_dt ASC";
        } else if (type == DataSource::StepSummary) {
            qry = "SELECT date(timestamp_dt), sum(steps) FROM mi_band_activity WHERE date(timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' GROUP BY date(timestamp_dt) ORDER BY timestamp_dt ASC";
        }

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
                        pt["y"] = h.toDouble();
                        result.append(pt);

                        curs->moveNext();
                    }
                    //qDebug() << result;
                }
                m_conn->deleteCursor(curs);
            } else {
                qDebug() << "Error executing query";
            }
        }
    }
    return result;
}
