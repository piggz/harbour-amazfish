#include "activitycoordinate.h"

ActivityCoordinate::ActivityCoordinate()
{
    
}
    
void ActivityCoordinate::setCoordinate(const QGeoCoordinate &coordinate)
{
    m_coordinate = coordinate;
}

QGeoCoordinate ActivityCoordinate::coordinate()
{
    return m_coordinate;
}
    
void ActivityCoordinate::setTimeStamp(const QDateTime& timeStamp)
{
    m_timeStamp = timeStamp;
}
QDateTime ActivityCoordinate::timeStamp()
{
    return m_timeStamp;
}
    
void ActivityCoordinate::setHeartRate(int hr)
{
    m_heartRate = hr;
}

int ActivityCoordinate::heartRate()
{
    return m_heartRate;
}

bool ActivityCoordinate::isValid()
{
    return m_isValid;
}