#ifndef ACTIVITYSUMMARY_H
#define ACTIVITYSUMMARY_H

#include <QObject>
#include <QDateTime>
#include "activitykind.h"

class ActivitySummary
{

public:
    ActivitySummary();

    void setVersion(short);
    short version() const;

    void setId(uint);
    uint id() const;
    
    void setName(const QString& n);
    QString name() const;

    void setActivityKind(ActivityKind::Type);
    ActivityKind::Type activityKind() const;
            
    void setStartTime(const QDateTime& start);
    QDateTime startTime() const;
    
    void setEndTime(const QDateTime& end);
    QDateTime endTime() const;

    void setBaseLatitude(int lat);
    int baseLatitude() const;

    void setBaseLongitude(int lon);
    int baseLongitude() const;

    void setBaseAltitude(int alt);
    int baseAltitude() const;

private:
    short m_version;
    uint m_id;
    QString m_name;
    QDateTime m_startTime;     /** Not-null value. */
    QDateTime m_endTime;     /** Not-null value. */
    ActivityKind::Type m_activityKind;
    int m_baseLongitude;
    int m_baseLatitude;
    int m_baseAltitude;
    QString m_gpxTrack;
    uint m_deviceId;
    uint m_userId;
};

#endif // ACTIVITYSUMMARY_H
