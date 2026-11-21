#ifndef ACTIVITYSUMMARY_H
#define ACTIVITYSUMMARY_H

#include <QObject>
#include <QDateTime>
#include <QList>

#include "activitykind.h"

class ActivitySummary
{

public:
    ActivitySummary();

    struct meta {
        QString key;
        QString value;
        QString unit;
    };

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

    void setBaseLatitude(float lat);
    float baseLatitude() const;

    void setBaseLongitude(float lon);
    float baseLongitude() const;

    void setBaseAltitude(int alt);
    int baseAltitude() const;

    // False when the watch gave no location (no GPS); the database then stores NULL.
    bool hasBaseLocation() const;

    void setProfileId(uint profileId);
    uint profileId() const;

    void setDeviceId(uint deviceId);
    uint deviceId() const;

    void setGPX(const QString &gpx);
    QString gpx() const;

    void addMetaData(const QString &key, const QString &value, const QString &unit);
    ActivitySummary::meta metaData(const QString &key);
    QList<ActivitySummary::meta> metaDataList() const;

    bool isValid() const;
    void setValid(bool valid);

private:
    short m_version = 0;
    uint m_id = 0;
    QString m_name;
    QDateTime m_startTime;     /** Not-null value. */
    QDateTime m_endTime;     /** Not-null value. */
    ActivityKind::Type m_activityKind = ActivityKind::Unknown;
    float m_baseLongitude = 0;
    float m_baseLatitude = 0;
    int m_baseAltitude = 0;
    bool m_hasBaseLocation = false;
    QString m_gpxTrack;
    uint m_deviceId = 0;
    uint m_userId = 0;

    bool m_valid = false;



    QList<meta> m_metaData;
};

#endif // ACTIVITYSUMMARY_H
