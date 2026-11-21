#include "activitysummary.h"
#include "database.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

ActivitySummary::ActivitySummary()
{

}

void ActivitySummary::setId(uint id)
{
    m_id = id;
}

uint ActivitySummary::id() const
{
    return m_id;
}

void ActivitySummary::setName(const QString &n)
{
    m_name = n;
}

QString ActivitySummary::name() const
{
    return m_name;
}

void ActivitySummary::setVersion(short v)
{
    m_version=v;
}

short ActivitySummary::version() const
{
    return m_version;
}

void ActivitySummary::setActivityKind(ActivityKind::Type type)
{
    m_activityKind = type;
}

ActivityKind::Type ActivitySummary::activityKind() const
{
    return m_activityKind;
}   

void ActivitySummary::setStartTime(const QDateTime& start)
{
    m_startTime = start;
}

QDateTime ActivitySummary::startTime() const
{
    return m_startTime;
}

void ActivitySummary::setEndTime(const QDateTime& end)
{
    m_endTime = end;
}

QDateTime ActivitySummary::endTime() const
{
    return m_endTime;
}

void ActivitySummary::setBaseLatitude(float lat)
{
    m_baseLatitude = lat;
}

float ActivitySummary::baseLatitude() const
{
    return m_baseLatitude;
}

void ActivitySummary::setBaseLongitude(float lon)
{
    m_baseLongitude = lon;
}

float ActivitySummary::baseLongitude() const
{
    return m_baseLongitude;
}

void ActivitySummary::setBaseAltitude(int alt)
{
    m_baseAltitude = alt;
}

int ActivitySummary::baseAltitude() const
{
    return m_baseAltitude;
}

void ActivitySummary::setProfileId(uint profileId)
{
    m_userId = profileId;
}

void ActivitySummary::setDeviceId(uint deviceId)
{
    m_deviceId = deviceId;
}

void ActivitySummary::setGPX(const QString &gpx)
{
    m_gpxTrack = gpx;
}

void ActivitySummary::addMetaData(const QString &key, const QString &value, const QString &unit)
{
    meta m;
    m.key = key;
    m.value = value;
    m.unit = unit;
    m_metaData << m;
}

ActivitySummary::meta ActivitySummary::metaData(const QString &key)
{
    ActivitySummary::meta m;

    for(const auto &i:std::as_const(m_metaData)) {
        if (i.key == key) {
            m = i;
            break;
        }
    }
    return m;
}

bool ActivitySummary::isValid() const
{
    return m_valid;
}

void ActivitySummary::setValid(bool valid)
{
    m_valid = valid;
}

bool ActivitySummary::saveToDatabase()
{
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO sports_data (name, version, start_timestamp, start_timestamp_dt, end_timestamp, "
                  "end_timestamp_dt, device_id, user_id, kind, base_longitude, base_latitude, base_altitude, gpx) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(name());
    query.addBindValue(version());
    query.addBindValue(startTime().toMSecsSinceEpoch() / 1000);
    query.addBindValue(Database::dateTimeToText(startTime()));
    query.addBindValue(endTime().toMSecsSinceEpoch() / 1000);
    query.addBindValue(Database::dateTimeToText(endTime()));
    query.addBindValue(m_deviceId);
    query.addBindValue(m_userId);
    query.addBindValue(activityKind());
    query.addBindValue(baseLongitude());
    query.addBindValue(baseLatitude());
    query.addBindValue(baseAltitude());
    query.addBindValue(m_gpxTrack);

    // qDebug() << Q_FUNC_INFO << "Saving Sport:" << name()  << startTime() << startTime().toString(Qt::ISODate);

    if (!query.exec()) {
        qWarning() << Q_FUNC_INFO << "Error inserting record" << query.lastError().text();
        db.rollback();
        return false;
    }

    long lastId = query.lastInsertId().toLongLong();
    setId(lastId);
    // qDebug() << Q_FUNC_INFO << "Record Id is" << id();

    //Save meta data
    query.prepare("INSERT INTO sports_meta (sport_id, key, value, unit) VALUES (?, ?, ?, ?)");
    foreach(const auto &m, m_metaData) {
        query.addBindValue(QVariant::fromValue(lastId));
        query.addBindValue(m.key);
        query.addBindValue(m.value);
        query.addBindValue(m.unit);

        if (!query.exec()) {
            qWarning() << Q_FUNC_INFO << "Error inserting meta record" << query.lastError().text();
            db.rollback();
            return false;
        }
    }
    //Done

    if (!db.commit()) {
        qWarning() << Q_FUNC_INFO << "Cannot save the activity:" << db.lastError().text();
        return false;
    }
    return true;
}
