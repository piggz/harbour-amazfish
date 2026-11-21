#include "activitysummary.h"

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
    m_hasBaseLocation = true;
}

float ActivitySummary::baseLatitude() const
{
    return m_baseLatitude;
}

void ActivitySummary::setBaseLongitude(float lon)
{
    m_baseLongitude = lon;
    m_hasBaseLocation = true;
}

float ActivitySummary::baseLongitude() const
{
    return m_baseLongitude;
}

void ActivitySummary::setBaseAltitude(int alt)
{
    m_baseAltitude = alt;
    m_hasBaseLocation = true;
}

int ActivitySummary::baseAltitude() const
{
    return m_baseAltitude;
}

bool ActivitySummary::hasBaseLocation() const
{
    return m_hasBaseLocation;
}

void ActivitySummary::setProfileId(uint profileId)
{
    m_userId = profileId;
}

uint ActivitySummary::profileId() const
{
    return m_userId;
}

void ActivitySummary::setDeviceId(uint deviceId)
{
    m_deviceId = deviceId;
}

uint ActivitySummary::deviceId() const
{
    return m_deviceId;
}

void ActivitySummary::setGPX(const QString &gpx)
{
    m_gpxTrack = gpx;
}

QString ActivitySummary::gpx() const
{
    return m_gpxTrack;
}

void ActivitySummary::addMetaData(const QString &key, const QString &value, const QString &unit)
{
    meta m;
    m.key = key;
    m.value = value;
    m.unit = unit;
    m_metaData << m;
}

QList<ActivitySummary::meta> ActivitySummary::metaDataList() const
{
    return m_metaData;
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
