#ifndef ACTIVITYCOORDINATE_H
#define ACTIVITYCOORDINATE_H

#include <QGeoCoordinate>
#include <QDateTime>

class ActivityCoordinate
{
public:
   ActivityCoordinate();
    
    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate() const;
    
    void setTimeStamp(const QDateTime& timeStamp);
    QDateTime timeStamp() const;
    
    void setHeartRate(int hr);
    int heartRate() const;
    
    bool operator==(const ActivityCoordinate &other) const;
    bool operator!=(const ActivityCoordinate &other) const;

private:
    
    QGeoCoordinate m_coordinate;
    QDateTime m_timeStamp;
    int m_heartRate = 0;    
};

#endif
