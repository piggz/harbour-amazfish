#ifndef ACTIVITYSUMMARY_H
#define ACTIVITYSUMMARY_H

#include <QObject>
#include <QDateTime>

class ActivitySummary : public QObject
{
    Q_OBJECT
public:
    ActivitySummary();
    Q_PROPERTY(short version MEMBER m_version)
    Q_PROPERTY(uint id MEMBER m_id)

private:
    short m_version;
    uint m_id;
    QString m_name;
    QDateTime m_startTime;     /** Not-null value. */
    QDateTime m_endTime;     /** Not-null value. */
    uint m_activityKind;
    int m_baseLongitude;
    int m_baseLatitude;
    int m_baseAltitude;
    QString m_gpxTrack;
    uint m_deviceId;
    uint m_userId;
};

#endif // ACTIVITYSUMMARY_H
