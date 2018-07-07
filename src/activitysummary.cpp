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
