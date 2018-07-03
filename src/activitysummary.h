#ifndef ACTIVITYSUMMARY_H
#define ACTIVITYSUMMARY_H

#include <QObject>
#include <QDateTime>
#include "operations/activitykind.h"

class ActivitySummary : public QObject
{
    Q_OBJECT
public:
    ActivitySummary();

    void setVersion(short);
    short version();

    void setId(uint);
    uint id();
    
    void setName(const QString& n);
    QString name();

    void setActivityKind(ActivityKind::Type);
    ActivityKind::Type activitKind();
            
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
