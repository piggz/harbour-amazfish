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

void ActivityCoordinate::setCadence(short cadence)
{
    m_cadence = cadence;
}

short ActivityCoordinate::cadence() const
{
    return m_cadence;
}

void ActivityCoordinate::setSpeed(float speed)
{
    m_speed = speed;
}

float ActivityCoordinate::speed() const
{
    return m_speed;
}

bool ActivityCoordinate::operator ==(const ActivityCoordinate &other) const
{
    return heartRate() == other.heartRate() && timeStamp() == other.timeStamp() && coordinate() == other.coordinate();
}

bool ActivityCoordinate::operator !=(const ActivityCoordinate &other) const
{
    return !(operator ==(other));
}
