#include "activitysummary.h"
#include <KDb3/KDbTransactionGuard>

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

void ActivitySummary::setBaseLatitude(int lat)
{
    m_baseLatitude = lat;
}

int ActivitySummary::baseLatitude() const
{
    return m_baseLatitude;
}

void ActivitySummary::setBaseLongitude(int lon)
{
    m_baseLongitude = lon;
}

int ActivitySummary::baseLongitude() const
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

    for(auto i:m_metaData) {
        if (i.key == key) {
            m = i;
            break;
        }
    }
    return m;
}

bool ActivitySummary::saveToDatabase(KDbConnection *conn)
{

    KDbTransaction transaction = conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto sportsData = conn->tableSchema("sports_data");

    fields.addField(sportsData->field("name"));
    fields.addField(sportsData->field("version"));
    fields.addField(sportsData->field("start_timestamp"));
    fields.addField(sportsData->field("start_timestamp_dt"));
    fields.addField(sportsData->field("end_timestamp"));
    fields.addField(sportsData->field("end_timestamp_dt"));
    fields.addField(sportsData->field("device_id"));
    fields.addField(sportsData->field("user_id"));
    fields.addField(sportsData->field("kind"));
    fields.addField(sportsData->field("base_longitude"));
    fields.addField(sportsData->field("base_latitude"));
    fields.addField(sportsData->field("base_altitude"));
    fields.addField(sportsData->field("gpx"));


    QList<QVariant> values;
    values << name();
    values << version();
    values << startTime().toMSecsSinceEpoch() / 1000;
    values << startTime();
    values << endTime().toMSecsSinceEpoch() / 1000;
    values << endTime();
    values << m_deviceId;
    values << m_userId;
    values << activityKind();
    values << baseLongitude() / 3000000.0;
    values << baseLatitude() / 3000000.0;
    values << baseAltitude();
    values << m_gpxTrack;

    qDebug() << Q_FUNC_INFO << "Saving Sport:" << name()  << startTime() << startTime().toString(Qt::ISODate);

    QSharedPointer<KDbSqlResult> result = conn->insertRecord(&fields, values);

    if (result->lastResult().isError()) {
        qDebug() << Q_FUNC_INFO << "Error inserting record";
        return false;
    }

    long lastId = result->lastInsertRecordId();
    setId(lastId);
    qDebug() << Q_FUNC_INFO << "Record Id is" << id();

    //Save meta data
    foreach(const auto &m, m_metaData) {
        KDbFieldList metaFields;
        auto metaData = conn->tableSchema("sports_meta");

        metaFields.addField(metaData->field("sport_id"));
        metaFields.addField(metaData->field("key"));
        metaFields.addField(metaData->field("value"));
        metaFields.addField(metaData->field("unit"));

        QList<QVariant> metaValues;
        metaValues << QVariant::fromValue(lastId);
        metaValues << m.key;
        metaValues << m.value;
        metaValues << m.unit;

        result = conn->insertRecord(&metaFields, metaValues);

        if (result->lastResult().isError()) {
            qDebug() << Q_FUNC_INFO << "Error inserting meta record";
            return false;
        }
    }
    //Done

    tg.commit();
    return true;
}
