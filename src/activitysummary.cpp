#include "activitysummary.h"

ActivitySummary::ActivitySummary()
{

}

void ActivitySummary::setVersion(short v)
{
    m_version=v;
}

void ActivitySummary::setActivityKind(ActivityKind::Type type)
{
    m_activityKind = type;
}


ActivityKind::Type ActivitySummary::activitKind()
{
    return m_activityKind;
}   


void ActivitySummary::setStartTime(const QDateTime& start)
{
    m_startTime = start;
}

void ActivitySummary::setEndTime(const QDateTime& end)
{
    m_endTime = end;
}
    
void ActivitySummary::setBaseLatitude(int lat)
{
    m_baseLatitude = lat;
}

void ActivitySummary::setBaseLongitude(int lon)
{
    m_baseLongitude = lon;
}
    
void ActivitySummary::setBaseAltitude(int alt)
{
    m_baseAltitude = alt;
}
    