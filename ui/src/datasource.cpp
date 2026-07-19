#include "datasource.h"
#include "amazfish.h"
#include <QVariant>
#include <QTimeZone>

DataSource::DataSource()
{
    qDebug() << Q_FUNC_INFO;
}

void DataSource::setConnection(KDbConnection *conn)
{
    qDebug() << Q_FUNC_INFO;
    m_conn = conn;
}

QVariant DataSource::data(const Type type, const QDate &day)
{
    qDebug() << Q_FUNC_INFO << type << day;
    QList<QVariant> result;

    QString qry;

    QDateTime sd;
    sd.setDate(day);
    sd.setTime(QTime(0,0));
    sd.setTimeZone(QTimeZone::systemTimeZone());

    if (type == DataSource::SleepSummary) {
        for (int i = 0; i < 10; i++) {
            QList<DataSource::SleepSession> sessions = calculateSleep(day.addDays(-10 + i));
            QVariantMap pt;
            pt["x"] = QDateTime(day.addDays(-10 + i)).toMSecsSinceEpoch() / 1000;

            float light = 0;
            float deep = 0;
            for (const DataSource::SleepSession &s : std::as_const(sessions)) {
                light += s.lightSleepDuration;
                deep += s.deepSleepDuration;;
            }
            pt["y"] = light / (60.0f*60.0f);
            pt["z"] = deep / (60.0f*60.0f);

            result.append(pt);
        }
        // qDebug() << "Sleep data:" << result;

#if 0
        qry = "SELECT timestamp_dt, raw_kind, raw_intensity FROM mi_band_activity WHERE timestamp_dt >= date('" +
                day.toString("yyyy-MM-ddT11:59:00") + "','-10 day') AND timestamp_dt <= '" +
                day.toString("yyyy-MM-ddT12:01:00") +  "' ORDER BY timestamp_dt ASC";

        // qDebug() << qry;
        if (m_conn && m_conn->isDatabaseUsed()) {
            KDbCursor *curs = m_conn->executeQuery(KDbEscapedString(qry));

            if (curs) {
                if (curs->open() && curs->moveFirst()) {
                    QDate d; //Current day, sleepd detection runs from midday to midday
                    int light = 0; //minutes in light sleep
                    int deep = 0; //mintes in deep sleep
                    QDate curDate = curs->value(0).toDateTime().date();
                    d = curDate.addDays(-1);

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

                        if (in_sleep && (activity == 112 || activity == 121 || activity == 122)) {
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
#endif
    } else {
        if (type == DataSource::Heartrate) {
            qry = "SELECT timestamp_dt, heartrate, timestamp FROM mi_band_activity WHERE heartrate < 255 AND timestamp >= '" +
                    QString::number(sd.toMSecsSinceEpoch() / 1000) +  "' AND timestamp <= '" +
                    QString::number((sd.toMSecsSinceEpoch() / 1000) + 86400)  + "' ORDER BY timestamp ASC";
        } else if (type == DataSource::Steps) {
            qry = "SELECT timestamp_dt, steps, timestamp FROM mi_band_activity WHERE timestamp >= '" +
                    QString::number(sd.toMSecsSinceEpoch() / 1000) +  "' AND timestamp <= '" +
                    QString::number((sd.toMSecsSinceEpoch() / 1000) + 86400) +  "' ORDER BY timestamp ASC";
        } else if (type == DataSource::Intensity) {
            qry = "SELECT timestamp_dt, ((raw_intensity / 255.0) * 100), timestamp FROM mi_band_activity WHERE timestamp >= '" +
                    QString::number(sd.toMSecsSinceEpoch() / 1000) +  "' AND timestamp <= '" +
                    QString::number((sd.toMSecsSinceEpoch() / 1000) + 86400) +  "' ORDER BY timestamp ASC";
        } else if (type == DataSource::StepSummary) {
            qry = "SELECT date(timestamp_dt), sum(steps) FROM mi_band_activity WHERE date(timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' GROUP BY date(timestamp_dt) ORDER BY timestamp_dt ASC";
        } else if (type == DataSource::BatteryLog) {
            qry = "SELECT timestamp_dt, value FROM info_log WHERE date(timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' AND key = '"+ QString::number((int)Amazfish::Info::INFO_BATTERY) +"' ORDER BY timestamp_dt ASC"; // 7 = AbstractDevice::INFO_BATTERY
        } else if (type == DataSource::HRV) {
            qry = "SELECT hrv_timestamp_dt, hrv_value FROM hrv WHERE hrv_timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") + "' AND hrv_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' ORDER BY hrv_timestamp_dt ASC";
        } else if (type == DataSource::Spo2Normal) {
            qry = "SELECT spo2_timestamp_dt, spo2_value FROM spo2 WHERE date(spo2_timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND spo2_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' GROUP BY date(spo2_timestamp_dt) ORDER BY spo2_timestamp_dt ASC";
        } else if (type == DataSource::Spo2Sleep) {
            qry = "SELECT spo2sleep_timestamp_dt, spo2sleep_value FROM spo2sleep WHERE date(spo2sleep_timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND spo2sleep_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' GROUP BY date(spo2sleep_timestamp_dt) ORDER BY spo2sleep_timestamp_dt ASC";
        } else if (type == DataSource::BodyTemperature) {
            qry = "SELECT temperature_timestamp_dt, temperature_value FROM temperature WHERE temperature_timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") + "' AND temperature_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' AND temperature_type = 'skin' AND temperature_location = 'wrist'" +
                    " ORDER BY temperature_timestamp_dt ASC";
        } else if (type == DataSource::StressAuto) {
            qry = "SELECT stress_timestamp_dt, stress_value FROM stress WHERE stress_type = 1 AND stress_timestamp_dt >= '" +
                    day.toString("yyyy-MM-ddT00:00:00") + "' AND stress_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' AND stress_type = 1" +
                    " ORDER BY stress_timestamp_dt ASC";
        } else if (type == DataSource::StressSummary) {
            qry = "SELECT date(stress_timestamp_dt), sum(stress_value) / count(stress_value) FROM stress WHERE date(stress_timestamp_dt) >= date('" +
                    day.toString("yyyy-MM-ddT00:00:00") + "','-10 day') AND stress_timestamp_dt <= '" +
                    day.toString("yyyy-MM-ddT23:59:59") +  "' GROUP BY date(stress_timestamp_dt) ORDER BY date(stress_timestamp_dt) ASC";
        } else if (type == DataSource::StressManual) { //Latest Manual reading
            qry = "SELECT stress_timestamp_dt, stress_value FROM stress WHERE stress_type = 2"
                    " ORDER BY stress_timestamp_dt DESC LIMIT 1";
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

QList<DataSource::SleepSession> DataSource::calculateSleep(const QDate &day)
{
    return QList<DataSource::SleepSession>();
}
