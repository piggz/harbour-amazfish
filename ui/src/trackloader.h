/*
 * Copyright (C) 2017 Jens Drescher, Germany
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TRACKLOADER_H
#define TRACKLOADER_H

#include <QObject>
#include <QDateTime>
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#define HEARTRATE_MIN_INIT 9999999
#define HEARTRATE_MAX_INIT 0

class TrackLoader : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filename READ filename WRITE setFilename NOTIFY filenameChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString workout READ workout NOTIFY workoutChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QDateTime time READ time NOTIFY timeChanged)
    Q_PROPERTY(QString timeStr READ timeStr NOTIFY timeChanged)
    Q_PROPERTY(int duration READ duration NOTIFY durationChanged)
    Q_PROPERTY(QString durationStr READ durationStr NOTIFY durationChanged)

    Q_PROPERTY(int pauseDuration READ pauseDuration NOTIFY durationChanged)
    Q_PROPERTY(QString pauseDurationStr READ pauseDurationStr NOTIFY durationChanged)

    Q_PROPERTY(qreal distance READ distance NOTIFY distanceChanged)
    Q_PROPERTY(qreal speed READ speed NOTIFY speedChanged)
    Q_PROPERTY(qreal maxSpeed READ maxSpeed NOTIFY maxSpeedChanged)
    Q_PROPERTY(qreal pace READ pace NOTIFY paceChanged)
    Q_PROPERTY(QString paceStr READ paceStr NOTIFY paceChanged)
    Q_PROPERTY(QString paceImperialStr READ paceImperialStr NOTIFY paceChanged)
    Q_PROPERTY(qreal heartRate READ heartRate NOTIFY heartRateChanged)
    Q_PROPERTY(uint heartRateMin READ heartRateMin NOTIFY heartRateMinChanged)
    Q_PROPERTY(uint heartRateMax READ heartRateMax NOTIFY heartRateMaxChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(qreal elevationUp READ elevationUp NOTIFY elevationChanged)
    Q_PROPERTY(qreal elevationDown READ elevationDown NOTIFY elevationChanged)

public:
    struct TrackPoint
    {
        qreal latitude;
        qreal longitude;
        QDateTime time;
        qreal elevation;
        qreal direction;
        qreal groundSpeed;
        qreal verticalSpeed;
        qreal magneticVariation;
        qreal horizontalAccuracy;
        qreal verticalAccuracy;
        uint heartrate;
        qreal distance;
        qreal speed;
        qreal pace;
        qreal duration;
    };

    explicit TrackLoader(QObject *parent = 0);
    QString filename() const;
    void setFilename(const QString &filename);
    Q_INVOKABLE void loadString(const QString &gpx);
    QString name();
    QString workout();
    QString description();
    QDateTime time();
    QString timeStr();
    uint duration();
    QString durationStr();
    uint pauseDuration();
    QString pauseDurationStr();
    qreal distance();
    qreal speed();
    qreal maxSpeed();
    qreal pace();
    QString paceStr();    
    QString paceImperialStr();
    qreal heartRate();
    uint heartRateMin();
    uint heartRateMax();
    qreal elevationUp();
    qreal elevationDown();
    bool loaded();

    Q_INVOKABLE QString readGpx();
    Q_INVOKABLE QString sTworkoutKey();
    Q_INVOKABLE int trackPointCount();
    Q_INVOKABLE int pausePositionsCount();
    Q_INVOKABLE QGeoCoordinate trackPointAt(int index);
    Q_INVOKABLE int pausePositionAt(int index);
    Q_INVOKABLE uint heartRateAt(int index);
    Q_INVOKABLE qreal elevationAt(int index);
    Q_INVOKABLE QDateTime timeAt(int index);
    Q_INVOKABLE qint64 unixTimeAt(int index);
    Q_INVOKABLE qreal durationAt(int index);
    Q_INVOKABLE qreal distanceAt(int index);
    Q_INVOKABLE qreal speedAt(int index);
    Q_INVOKABLE qreal paceAt(int index);
    Q_INVOKABLE QString paceStrAt(int index);
    Q_INVOKABLE QString paceImperialStrAt(int index);

    // Temporary "hacks" to get around misbehaving Map.fitViewportToMapItems()
    Q_INVOKABLE int fitZoomLevel(int width, int height);
    Q_INVOKABLE QGeoCoordinate center();

    Q_INVOKABLE void vReadFile(QString sFilename);
    Q_INVOKABLE void vSetNewProperties(QString sOldName, QString sOldDesc, QString sOldWorkout, QString sName, QString sDesc, QString sWorkout);
    Q_INVOKABLE void vWriteFile(QString sFilename);

    Q_INVOKABLE bool hasHeartRateData() const;
    Q_INVOKABLE bool paceRelevantForWorkoutType() const;

signals:
    void filenameChanged();
    void nameChanged();
    void workoutChanged();
    void descriptionChanged();
    void timeChanged();
    void durationChanged();
    void distanceChanged();
    void speedChanged();
    void maxSpeedChanged();
    void paceChanged();
    void loadedChanged();
    void trackChanged();
    void heartRateChanged();
    void heartRateMinChanged();
    void heartRateMaxChanged();
    void elevationChanged();

public slots:

private:
    void load();
    QString paceToStr(qreal pace);
    void parseXmlStream(QXmlStreamReader &xml);
    void parseXmlTcxStream(QXmlStreamReader &xml);

    QList<TrackPoint> m_points;
    QList<int> m_pause_positions;
    QList<QString> sFileStringArray;
    bool m_loaded;
    bool m_error;
    QString m_filename;
    QString m_name;
    QString m_workout;
    QString m_description;
    QDateTime m_time;
    uint m_duration;
    uint m_pause_duration;
    qreal m_distance;
    qreal m_speed;
    qreal m_maxSpeed;
    qreal m_pace;
    qreal m_heartRate;
    qreal m_heartRatePoints;
    uint m_heartRateMin;
    uint m_heartRateMax;
    QString m_sTkey; //Sports-Tracker.com workout key
    QGeoCoordinate m_center;
    qreal m_elevationUp;
    qreal m_elevationDown;
    QList<qreal> m_distancearray;
    QList<qreal> m_durationarray;
};

#endif // TRACKLOADER_H
