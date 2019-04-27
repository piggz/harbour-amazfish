#include "activitycoordinate.h"

ActivityCoordinate::ActivityCoordinate()
{
    
}
    
void ActivityCoordinate::setCoordinate(const QGeoCoordinate &coordinate)
{
    m_coordinate = coordinate;
}

QGeoCoordinate ActivityCoordinate::coordinate() const
{
    return m_coordinate;
}
    
void ActivityCoordinate::setTimeStamp(const QDateTime& timeStamp)
{
    m_timeStamp = timeStamp;
}

QDateTime ActivityCoordinate::timeStamp() const
{
    return m_timeStamp;
}
    
void ActivityCoordinate::setHeartRate(int hr)
{
    m_heartRate = hr;
}

int ActivityCoordinate::heartRate() const
{
    return m_heartRate;
}

bool ActivityCoordinate::isValid() const
{
    return m_isValid;
}

bool ActivityCoordinate::operator ==(const ActivityCoordinate &other) const
{
    return heartRate() == other.heartRate() && timeStamp() == other.timeStamp() && coordinate() == other.coordinate() && isValid() == other.isValid();
}

bool ActivityCoordinate::operator !=(const ActivityCoordinate &other) const
{
    return !(operator ==(other));
}
