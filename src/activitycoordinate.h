#ifndef ACTIVITYCOORDINATE_H
#define ACTIVITYCOORDINATE_H

#include <QGeoCoordinate>
#include <QDateTime>

class ActivityCoordinate
{
public:
   ActivityCoordinate();
    
    void setCoordinate(const QGeoCoordinate &coordinate);
    QGeoCoordinate coordinate();
    
    void setTimeStamp(const QDateTime& timeStamp);
    QDateTime timeStamp();
    
    void setHeartRate(int hr);;
    int heartRate();
    
    bool isValid();
private:
    
    QGeoCoordinate m_coordinate;
    QDateTime m_timeStamp;
    int m_heartRate = 0;    
    bool m_isValid = false;
};

#endif