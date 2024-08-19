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

#include <QStandardPaths>
#include <QFile>
#include <QSaveFile>
#include <QXmlStreamWriter>
#include <QGeoCoordinate>
#include <QDebug>
#include <qmath.h>

#include "timeformatter.h"
#include "trackloader.h"

TrackLoader::TrackLoader(QObject *parent) :
    QObject(parent)
{
}

QString TrackLoader::readGpx(){
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Laufhelden";
    QString fullFilename = dirName + "/" + m_filename;
    qDebug()<<"Reading File:"<<fullFilename;

    QFile f(fullFilename);
    if (!f.open(QFile::ReadOnly | QFile::Text)) return "";
    QTextStream in(&f);
    return in.readAll();
}

//Returns Sports-Tracker.com unique workoutkey.
QString TrackLoader::sTworkoutKey(){
    return m_sTkey;
}

void TrackLoader::vReadFile(const QString &sFilename)
{
    this->sFileStringArray.clear();

    QString dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Laufhelden";
    QString fullFilename = dirName + "/" + sFilename;
    qDebug()<<"Reading File:"<<fullFilename;

    QFile f(fullFilename);
    if (!f.open(QFile::ReadWrite | QFile::Text)) return;
    QTextStream in(&f);

    while (!in.atEnd())
    {
        this->sFileStringArray.append(in.readLine());
    }

    f.close();
}

void TrackLoader::vSetNewProperties(const QString &sOldName, const QString &sOldDesc, const QString &sOldWorkout, const QString &sName, const QString &sDesc, const QString &sWorkout)
{
    //Search for a line
    bool bNameFound = false;
    bool bDescFound = false;
    bool bMeerunFound = false;

    for (int i = 0; i < this->sFileStringArray.length(); i++)
    {
        if (!bDescFound && this->sFileStringArray.at(i).contains(sOldDesc) && this->sFileStringArray.at(i).contains("<desc>", Qt::CaseInsensitive) && this->sFileStringArray.at(i).contains("</desc>", Qt::CaseInsensitive))
        {
            qDebug()<<"Found description: "<<this->sFileStringArray.at(i);
            bDescFound = true;

            this->sFileStringArray.replace(i, "        <desc>" + sDesc + "</desc>");
            /*
            //Extract description
            QString sDescription = this->sFileStringArray.at(i).trimmed();
            sDescription.chop(7);
            sDescription = sDescription.remove(0,6);

            qDebug()<<"Description: "<<sDescription;
            */
        }

        if (!bNameFound && this->sFileStringArray.at(i).contains(sOldName) && this->sFileStringArray.at(i).contains("<name>", Qt::CaseInsensitive) && this->sFileStringArray.at(i).contains("</name>", Qt::CaseInsensitive))
        {
            qDebug()<<"Found name: "<<this->sFileStringArray.at(i);
            bNameFound = true;

            this->sFileStringArray.replace(i, "        <name>" + sName + "</name>");
        }

        if (!bMeerunFound && this->sFileStringArray.at(i).contains("<meerun", Qt::CaseInsensitive) && this->sFileStringArray.at(i).contains("activity=", Qt::CaseInsensitive))
        {
            qDebug()<<"Found meerun: "<<this->sFileStringArray.at(i);
            bMeerunFound = true;

            QString sMeerun = this->sFileStringArray.at(i);
            int iActivityPosition = this->sFileStringArray.at(i).indexOf("activity=", 0);
            sMeerun = sMeerun.remove(iActivityPosition, 9 + 2 + sOldWorkout.length());
            //qDebug()<<"Meerun: "<<sMeerun;

            sMeerun = sMeerun.insert(iActivityPosition, "activity=\"" + sWorkout + "\"");
            //qDebug()<<"Meerun: "<<sMeerun;

            this->sFileStringArray.replace(i, sMeerun);
        }

        if (bDescFound && bNameFound && bMeerunFound)
            break;
    }
}

void TrackLoader::vWriteFile(const QString &sFilename)
{
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Laufhelden";
    QString fullFilename = dirName + "/" + sFilename;
    qDebug()<<"Writing File:"<<fullFilename;


    QFile fOut(fullFilename);
    if (fOut.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream s(&fOut);
        for (int i = 0; i < this->sFileStringArray.size(); ++i)
        {
            s << this->sFileStringArray.at(i) << '\n';
        }
    }
    else
    {
        qDebug() << "error opening output file\n";
        return;
    }
    fOut.close();
}

bool TrackLoader::hasHeartRateData() const
{
    return !(m_heartRateMin == HEARTRATE_MIN_INIT && m_heartRateMax == HEARTRATE_MAX_INIT);
}

/**
 * @brief TrackLoader::paceRelevantForWorkoutType
 * @return Returns false if the workout type is biking (pace is not relevant for this sport)
 */
bool TrackLoader::paceRelevantForWorkoutType() const
{
    return m_workout != "biking" && m_workout != "mountainBiking";
}

void TrackLoader::load()
{
    if(m_filename.isEmpty())
    {
        return;
    }
    QString dirName = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Laufhelden";
    QString fullFilename = dirName + "/" + m_filename;
    QFile file(fullFilename);

    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<"Error opening"<<fullFilename;
        m_error = true;
        return;
    }
}

void TrackLoader::loadString(const QString &gpx)
{
    QXmlStreamReader xml(gpx);

    if (gpx.contains("TrainingCenterDatabase")) {
        parseXmlTcxStream(xml);
    } else {
        parseXmlStream(xml);
    }
}

void TrackLoader::parseXmlStream(QXmlStreamReader &xml)
{
    if(!xml.readNextStartElement())
    {
        qDebug()<<m_filename<<"is not xml file?";
        m_error = true;
        return;
    }
    if( !(xml.name() == "gpx" && xml.attributes().value("version") == "1.1") )
    {
        qDebug()<<m_filename<<"is not gpx 1.1 file";
        m_error = true;
        return;
    }

    //count track segments
    int iSegments = 0;
    bool bPauseFound = false;


    // Loading considered succeeded at this point
    m_loaded = true;
    emit loadedChanged();

    while(!xml.atEnd())
    {
        while(xml.readNextStartElement())
        {
            if(xml.name() == "metadata")
            {
                while(xml.readNextStartElement())
                {
                    if(xml.name() == "name")
                    {
                        m_name = xml.readElementText();
                        emit nameChanged();
                    }
                    else if(xml.name() == "desc")
                    {
                        m_description = xml.readElementText();
                        emit descriptionChanged();
                    }
                    else if(xml.name() == "extensions")
                    {
                        while(xml.readNextStartElement())
                        {
                            if(xml.name() == "meerun")
                            {
                                m_workout = xml.attributes().value("activity").toString();
                                emit workoutChanged();
                            }
                            else if(xml.name() == "sportstracker")
                            {
                                m_sTkey = xml.attributes().value("workoutkey").toString();
                                m_workout = xml.attributes().value("activity").toString();
                                emit workoutChanged();
                            }
                            else
                            {
                                xml.skipCurrentElement();
                            }
                        }
                    }
                    else
                    {
                        xml.skipCurrentElement();
                    }
                }
            }
            else if(xml.name() == "trk")
            {
                while(xml.readNextStartElement())
                {
                    if(xml.name() == "trkseg")
                    {
                        //Count segments
                        iSegments++;

                        //If this is NOT the first segment, we are here after a pause in the track
                        if (iSegments > 1)
                        {
                            //mark that we have found a pause
                            bPauseFound = true;
                        }

                        while(xml.readNextStartElement())
                        {
                            if(xml.name() == "trkpt")
                            {
                                TrackPoint point;

                                point.elevation = 0;
                                point.direction = 0;
                                point.groundSpeed = 0;
                                point.verticalSpeed = 0;
                                point.magneticVariation = 0;
                                point.horizontalAccuracy = 0;
                                point.verticalAccuracy = 0;
                                point.heartrate = 0;
                                point.latitude = xml.attributes().value("lat").toDouble();
                                point.longitude = xml.attributes().value("lon").toDouble();
                                point.distance = 0;
                                point.speed = 0;
                                point.pace = 0;
                                point.duration = 0;

                                while(xml.readNextStartElement())
                                {
                                    if(xml.name() == "time") {
                                        point.time = QDateTime::fromString(xml.readElementText(),Qt::ISODate);
                                    } else if(xml.name() == "ele") {
                                        point.elevation = xml.readElementText().toDouble();
                                    } else if(xml.name() == "extensions") {
                                        while(xml.readNextStartElement())
                                        {
                                            if(xml.name() == "dir")
                                            {
                                                point.direction = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "g_spd")
                                            {
                                                point.groundSpeed = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "v_spd")
                                            {
                                                point.verticalSpeed = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "m_var")
                                            {
                                                point.magneticVariation = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "h_acc")
                                            {
                                                point.horizontalAccuracy = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "v_acc")
                                            {
                                                point.verticalAccuracy = xml.readElementText().toDouble();
                                            }
                                            else if(xml.name() == "TrackPointExtension")
                                            {
                                                while(xml.readNextStartElement())
                                                {
                                                    if(xml.name() == "hr")
                                                    {
                                                        point.heartrate = xml.readElementText().toInt();
                                                    }
                                                    else
                                                        xml.skipCurrentElement();
                                                }
                                            }
                                            else
                                                xml.skipCurrentElement();
                                        }
                                    }
                                }
                                //Before the point is appended we need to calculate a few more things.
                                //We need at least 2 points for that.
                                if(m_points.size() > 0)
                                {
                                    QDateTime firstTime(m_points.at(0).time);
                                    QDateTime secondTime(point.time);
                                    point.duration = firstTime.secsTo(secondTime);

                                    //If there was a pause before this trackpoint, do not calculate distance
                                    if (bPauseFound)
                                    {
                                        point.speed = 0;
                                        point.pace = 99;
                                        m_distancearray.clear();
                                        m_durationarray.clear();
                                    }
                                    else
                                    {
                                        //Fill distance array. Save the last few values to have a better speed/pace calculation.

                                        //Calculate distance in meter [m] between last and this point
                                        QGeoCoordinate first(m_points.at(m_points.size() - 1).latitude,m_points.at(m_points.size() - 1).longitude);
                                        QGeoCoordinate second(point.latitude,point.longitude);
                                        qreal rCurrentDistance = first.distanceTo(second);
                                        m_distance += rCurrentDistance;

                                        //Calculate time in seconds [s] between last and this point
                                        qreal rCurrentDuration = 0;
                                        QDateTime lastTime(m_points.at(m_points.size() - 1).time);
                                        rCurrentDuration = lastTime.secsTo(secondTime);

                                        //Fill distance array. Save the last few values to have a better speed/pace calculation.
                                        if (m_distancearray.length() == 10)
                                        {
                                            m_distancearray.removeFirst();
                                            m_durationarray.removeFirst();
                                        }
                                        m_distancearray.append(rCurrentDistance);
                                        m_durationarray.append(rCurrentDuration);

                                        rCurrentDistance = 0.0;
                                        rCurrentDuration = 0;
                                        //Calculate distance over the last few gps points
                                        for(int i=0 ; i < m_distancearray.length(); i++)
                                        {
                                            rCurrentDistance += m_distancearray[i];
                                            rCurrentDuration += m_durationarray[i];
                                        }

                                        //Calculate speed in [km/h]
                                        point.speed = (rCurrentDistance / 1000.0) / (rCurrentDuration / 3600.0);

                                        //Calculate pace in [min/km]
                                        point.pace = (rCurrentDuration / 60.0) / (rCurrentDistance / 1000.0);
                                    }
                                    point.distance = m_distance;
                                }

                                //if a pause is right before this track point, we have to save the index of this track point
                                if (bPauseFound)
                                {
                                    qDebug()<<"Pause found: "<<QString::number(m_points.length());

                                    m_pause_positions.append(m_points.length()-1);
                                    bPauseFound = false;
                                }

                                m_points.append(point);
                            }
                        }
                    }
                    else if(xml.name() == "trkpt")
                    {
                        TrackPoint point;

                        point.elevation = 0;
                        point.direction = 0;
                        point.groundSpeed = 0;
                        point.verticalSpeed = 0;
                        point.magneticVariation = 0;
                        point.horizontalAccuracy = 0;
                        point.verticalAccuracy = 0;
                        point.heartrate = 0;
                        point.latitude = xml.attributes().value("lat").toDouble();
                        point.longitude = xml.attributes().value("lon").toDouble();
                        point.distance = 0;
                        point.speed = 0;
                        point.pace = 0;
                        point.duration = 0;

                        while(xml.readNextStartElement())
                        {
                            if(xml.name() == "time") {
                                point.time = QDateTime::fromString(xml.readElementText(),Qt::ISODate);
                            } else if(xml.name() == "ele") {
                                point.elevation = xml.readElementText().toDouble();
                            } else if(xml.name() == "extensions") {
                                while(xml.readNextStartElement())
                                {
                                    if(xml.name() == "dir")
                                    {
                                        point.direction = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "g_spd")
                                    {
                                        point.groundSpeed = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "v_spd")
                                    {
                                        point.verticalSpeed = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "m_var")
                                    {
                                        point.magneticVariation = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "h_acc")
                                    {
                                        point.horizontalAccuracy = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "v_acc")
                                    {
                                        point.verticalAccuracy = xml.readElementText().toDouble();
                                    }
                                    else if(xml.name() == "TrackPointExtension")
                                    {
                                        while(xml.readNextStartElement())
                                        {
                                            if(xml.name() == "hr")
                                            {
                                                point.heartrate = xml.readElementText().toInt();
                                            }
                                            else
                                                xml.skipCurrentElement();
                                        }
                                    }
                                    else
                                        xml.skipCurrentElement();
                                }
                            }
                        }

                        //Before the point is appended we need to calculate a few more things.
                        //We need at least 2 points for that.
                        if(m_points.size() > 0)
                        {
                            QDateTime firstTime(m_points.at(0).time);
                            QDateTime secondTime(point.time);
                            point.duration = firstTime.secsTo(secondTime);

                            //If there was a pause before this trackpoint, do not calculate distance
                            if (bPauseFound)
                            {
                                point.speed = 0;
                                point.pace = 99;
                                m_distancearray.clear();
                                m_durationarray.clear();
                            }
                            else
                            {
                                //Fill distance array. Save the last few values to have a better speed/pace calculation.

                                //Calculate distance in meter [m] between last and this point
                                QGeoCoordinate first(m_points.at(m_points.size() - 1).latitude,m_points.at(m_points.size() - 1).longitude);
                                QGeoCoordinate second(point.latitude,point.longitude);
                                qreal rCurrentDistance = first.distanceTo(second);
                                m_distance += rCurrentDistance;

                                //Calculate time in seconds [s] between last and this point
                                qreal rCurrentDuration = 0;
                                QDateTime lastTime(m_points.at(m_points.size() - 1).time);
                                rCurrentDuration = lastTime.secsTo(secondTime);

                                //Fill distance array. Save the last few values to have a better speed/pace calculation.
                                if (m_distancearray.length() == 10)
                                {
                                    m_distancearray.removeFirst();
                                    m_durationarray.removeFirst();
                                }
                                m_distancearray.append(rCurrentDistance);
                                m_durationarray.append(rCurrentDuration);

                                rCurrentDistance = 0.0;
                                rCurrentDuration = 0;
                                //Calculate distance over the last few gps points
                                for(int i=0 ; i < m_distancearray.length(); i++)
                                {
                                    rCurrentDistance += m_distancearray[i];
                                    rCurrentDuration += m_durationarray[i];
                                }

                                //Calculate speed in [km/h]
                                point.speed = (rCurrentDistance / 1000.0) / (rCurrentDuration / 3600.0);

                                //Calculate pace in [min/km]
                                point.pace = (rCurrentDuration / 60.0) / (rCurrentDistance / 1000.0);
                            }
                            point.distance = m_distance;
                        }

                        //if a pause is right before this track point, we have to save the index of this track point
                        if (bPauseFound)
                        {
                            qDebug()<<"Pause found: "<<QString::number(m_points.length());

                            m_pause_positions.append(m_points.length()-1);
                            bPauseFound = false;
                        }

                        m_points.append(point);
                    }
                }
            }
            else
            {
                xml.skipCurrentElement();
            }
        }
    }

    qDebug()<<"Segments found: "<<QString::number(iSegments);

    if(m_points.size() > 1)
    {
        QDateTime firstTime(m_points.at(0).time);
        QDateTime secondTime(m_points.at(m_points.size()-1).time);
        m_duration = firstTime.secsTo(secondTime);

        m_time = firstTime.toLocalTime();

        qDebug()<<"m_distance first calculated: "<<QString::number(m_distance);

        m_distance = 0;

        int iPausePositionsIndex = 0;

        qreal rElevationLastValue = 0;

        for(int i=1;i<m_points.size();i++)
        {
            //We need to find out if this point is the end of a pause
            if (this->pausePositionsCount() > 0 && i==(this->pausePositionAt(iPausePositionsIndex) + 1))
            {
                qDebug()<<"Pause point: "<<QString::number(i);

                //Here we are at a point where a pause ends. We can calculate the pause duration here
                QDateTime firstTime(m_points.at(i-1).time);
                QDateTime secondTime(m_points.at(i).time);
                m_pause_duration = m_pause_duration + firstTime.secsTo(secondTime);

                if ((iPausePositionsIndex + 1) < this->pausePositionsCount())
                {
                    iPausePositionsIndex++;
                }
            }
            else
            {
                QGeoCoordinate first(m_points.at(i-1).latitude,m_points.at(i-1).longitude);
                QGeoCoordinate second(m_points.at(i).latitude,m_points.at(i).longitude);

                m_distance += first.distanceTo(second);
            }

            if(m_points.at(i).groundSpeed > m_maxSpeed)
            {
                m_maxSpeed = m_points.at(i).groundSpeed;
            }
            //If this point has a heart rate
            if (m_points.at(i).heartrate > 0)
            {
                m_heartRatePoints++;
                m_heartRate += m_points.at(i).heartrate;

                if (m_points.at(i).heartrate > m_heartRateMax)
                    m_heartRateMax = m_points.at(i).heartrate;
                if (m_points.at(i).heartrate < m_heartRateMin)
                    m_heartRateMin = m_points.at(i).heartrate;
            }

            //Elevation Up/Down
            if (i > 1)
            {
                if (m_points.at(i).elevation > rElevationLastValue)
                    m_elevationUp = m_elevationUp + (m_points.at(i).elevation - rElevationLastValue);

                if (m_points.at(i).elevation < rElevationLastValue)
                    m_elevationDown = m_elevationDown + (rElevationLastValue - m_points.at(i).elevation);
            }

            //Save this elevation value for next iteration
            rElevationLastValue = m_points.at(i).elevation;
        }

        //We need to substract the pause duration from the overall duration
        m_duration = m_duration - m_pause_duration;

        m_speed = m_distance / m_duration;
        m_pace = m_duration / m_distance * 1000 / 60;
        m_heartRate = m_heartRate / m_heartRatePoints;

        qDebug()<<"m_distance second calculated: "<<QString::number(m_distance);

        emit paceChanged();
        emit heartRateChanged();
        emit speedChanged();
        emit heartRateMinChanged();
        emit heartRateMaxChanged();
        emit distanceChanged();
        emit maxSpeedChanged();
        emit durationChanged();
        emit timeChanged();
        emit elevationChanged();
    }
    else
    {
        qDebug()<<"Not enough trackpoints to calculate duration, distance and speed";
        if(m_points.size() > 0)
        {
            QDateTime firstTime(m_points.at(0).time);
            m_time = firstTime.toLocalTime();
            emit timeChanged();
        }
    }
    emit trackChanged();
}


void TrackLoader::parseXmlTcxStream(QXmlStreamReader &xml)
{
    if(!xml.readNextStartElement())
    {
        qDebug()<<m_filename<<"is not xml file?";
        m_error = true;
        return;
    }
    if( !(xml.name() == "TrainingCenterDatabase" && xml.namespaceUri() == "http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2") )
    {
        qDebug()<<m_filename<<"is not TCX V2 file";
        m_error = true;
        return;
    }

    //count track segments
    int iSegments = 0;
    bool bPauseFound = false;


    // Loading considered succeeded at this point
    m_loaded = true;
    emit loadedChanged();

    while(!xml.atEnd())
    {
        while(xml.readNextStartElement())
        {
            if(xml.name() == "Activities")
            {
                while(xml.readNextStartElement())
                {
                    if(xml.name() == "Activity")
                    {
                        while(xml.readNextStartElement())
                        {
                            if(xml.name() == "Lap")
                            {
                                while(xml.readNextStartElement())
                                {
                                    if(xml.name() == "Track")
                                    {
                                        //Count segments
                                        iSegments++;

                                        //If this is NOT the first segment, we are here after a pause in the track
                                        if (iSegments > 1)
                                        {
                                            //mark that we have found a pause
                                            bPauseFound = true;
                                        }

                                        while(xml.readNextStartElement())
                                        {
                                            if(xml.name() == "Trackpoint")
                                            {
                                                TrackPoint point;

                                                point.elevation = 0;
                                                point.direction = 0;
                                                point.groundSpeed = 0;
                                                point.verticalSpeed = 0;
                                                point.magneticVariation = 0;
                                                point.horizontalAccuracy = 0;
                                                point.verticalAccuracy = 0;
                                                point.heartrate = 0;
                                                point.latitude = 0;
                                                point.longitude = 0;
                                                point.distance = 0;
                                                point.speed = 0;
                                                point.pace = 0;
                                                point.duration = 0;

                                                while(xml.readNextStartElement())
                                                {
                                                    if(xml.name() == "Time") {
                                                        point.time = QDateTime::fromString(xml.readElementText(),Qt::ISODate);
                                                    } else if(xml.name() == "AltitudeMeters") {
                                                        point.elevation = xml.readElementText().toDouble();
                                                    } else if(xml.name() == "HeartRateBpm") {
                                                        while(xml.readNextStartElement())
                                                        {
                                                            if(xml.name() == "Value") {
                                                                point.heartrate = xml.readElementText().toInt();
                                                            } else {
                                                                xml.skipCurrentElement();
                                                            }
                                                        }
                                                    } else if(xml.name() == "Position") {
                                                        while(xml.readNextStartElement())
                                                        {
                                                            if(xml.name() == "LatitudeDegrees") {
                                                                point.latitude = xml.readElementText().toDouble();
                                                            } else if (xml.name() == "LongitudeDegrees") {
                                                                point.longitude = xml.readElementText().toDouble();
                                                            } else {
                                                                xml.skipCurrentElement();
                                                            }
                                                        }
                                                    }
                                                }
                                                //Before the point is appended we need to calculate a few more things.
                                                //We need at least 2 points for that.
                                                if(m_points.size() > 0)
                                                {
                                                    QDateTime firstTime(m_points.at(0).time);
                                                    QDateTime secondTime(point.time);
                                                    point.duration = firstTime.secsTo(secondTime);

                                                    //If there was a pause before this trackpoint, do not calculate distance
                                                    if (bPauseFound)
                                                    {
                                                        point.speed = 0;
                                                        point.pace = 99;
                                                        m_distancearray.clear();
                                                        m_durationarray.clear();
                                                    }
                                                    else
                                                    {
                                                        //Fill distance array. Save the last few values to have a better speed/pace calculation.

                                                        //Calculate distance in meter [m] between last and this point
                                                        QGeoCoordinate first(m_points.at(m_points.size() - 1).latitude,m_points.at(m_points.size() - 1).longitude);
                                                        QGeoCoordinate second(point.latitude,point.longitude);
                                                        qreal rCurrentDistance = first.distanceTo(second);
                                                        m_distance += rCurrentDistance;

                                                        //Calculate time in seconds [s] between last and this point
                                                        qreal rCurrentDuration = 0;
                                                        QDateTime lastTime(m_points.at(m_points.size() - 1).time);
                                                        rCurrentDuration = lastTime.secsTo(secondTime);

                                                        //Fill distance array. Save the last few values to have a better speed/pace calculation.
                                                        if (m_distancearray.length() == 10)
                                                        {
                                                            m_distancearray.removeFirst();
                                                            m_durationarray.removeFirst();
                                                        }
                                                        m_distancearray.append(rCurrentDistance);
                                                        m_durationarray.append(rCurrentDuration);

                                                        rCurrentDistance = 0.0;
                                                        rCurrentDuration = 0;
                                                        //Calculate distance over the last few gps points
                                                        for(int i=0 ; i < m_distancearray.length(); i++)
                                                        {
                                                            rCurrentDistance += m_distancearray[i];
                                                            rCurrentDuration += m_durationarray[i];
                                                        }

                                                        //Calculate speed in [km/h]
                                                        point.speed = (rCurrentDistance / 1000.0) / (rCurrentDuration / 3600.0);

                                                        //Calculate pace in [min/km]
                                                        point.pace = (rCurrentDuration / 60.0) / (rCurrentDistance / 1000.0);
                                                    }
                                                    point.distance = m_distance;
                                                }

                                                //if a pause is right before this track point, we have to save the index of this track point
                                                if (bPauseFound)
                                                {
                                                    qDebug()<<"Pause found: "<<QString::number(m_points.length());

                                                    m_pause_positions.append(m_points.length()-1);
                                                    bPauseFound = false;
                                                }
                                                if (!(point.latitude == 0 && point.longitude == 0)) {
                                                    m_points.append(point);
                                                }
                                            } else {
                                                xml.skipCurrentElement();
                                            }
                                        }
                                    } else {
                                        xml.skipCurrentElement();
                                    }
                                }
                            } else if (xml.name() == "Id") {
                                qDebug() << "ID:" << xml.readElementText();
                            } else {
                                xml.skipCurrentElement();
                            }
                        }
                    }
                }
            }
        }
    }

    //qDebug()<<"Segments found: "<<QString::number(iSegments);

    if(m_points.size() > 1)
    {
        QDateTime firstTime(m_points.at(0).time);
        QDateTime secondTime(m_points.at(m_points.size()-1).time);
        m_duration = firstTime.secsTo(secondTime);

        m_time = firstTime.toLocalTime();

        qDebug()<<"m_distance first calculated: "<<QString::number(m_distance);

        m_distance = 0;

        int iPausePositionsIndex = 0;

        qreal rElevationLastValue = 0;

        for(int i=1;i<m_points.size();i++)
        {
            //We need to find out if this point is the end of a pause
            if (this->pausePositionsCount() > 0 && i==(this->pausePositionAt(iPausePositionsIndex) + 1))
            {
                qDebug()<<"Pause point: "<<QString::number(i);

                //Here we are at a point where a pause ends. We can calculate the pause duration here
                QDateTime firstTime(m_points.at(i-1).time);
                QDateTime secondTime(m_points.at(i).time);
                m_pause_duration = m_pause_duration + firstTime.secsTo(secondTime);

                if ((iPausePositionsIndex + 1) < this->pausePositionsCount())
                {
                    iPausePositionsIndex++;
                }
            }
            else
            {
                QGeoCoordinate first(m_points.at(i-1).latitude,m_points.at(i-1).longitude);
                QGeoCoordinate second(m_points.at(i).latitude,m_points.at(i).longitude);

                m_distance += first.distanceTo(second);
            }

            if(m_points.at(i).groundSpeed > m_maxSpeed)
            {
                m_maxSpeed = m_points.at(i).groundSpeed;
            }
            //If this point has a heart rate
            if (m_points.at(i).heartrate > 0)
            {
                m_heartRatePoints++;
                m_heartRate += m_points.at(i).heartrate;

                if (m_points.at(i).heartrate > m_heartRateMax)
                    m_heartRateMax = m_points.at(i).heartrate;
                if (m_points.at(i).heartrate < m_heartRateMin)
                    m_heartRateMin = m_points.at(i).heartrate;
            }

            //Elevation Up/Down
            if (i > 1)
            {
                if (m_points.at(i).elevation > rElevationLastValue)
                    m_elevationUp = m_elevationUp + (m_points.at(i).elevation - rElevationLastValue);

                if (m_points.at(i).elevation < rElevationLastValue)
                    m_elevationDown = m_elevationDown + (rElevationLastValue - m_points.at(i).elevation);
            }

            //Save this elevation value for next iteration
            rElevationLastValue = m_points.at(i).elevation;
        }

        //We need to substract the pause duration from the overall duration
        m_duration = m_duration - m_pause_duration;

        m_speed = m_distance / m_duration;
        m_pace = m_duration / m_distance * 1000 / 60;
        m_heartRate = m_heartRate / m_heartRatePoints;

        qDebug()<<"m_distance second calculated: "<<QString::number(m_distance);

        emit paceChanged();
        emit heartRateChanged();
        emit speedChanged();
        emit heartRateMinChanged();
        emit heartRateMaxChanged();
        emit distanceChanged();
        emit maxSpeedChanged();
        emit durationChanged();
        emit timeChanged();
        emit elevationChanged();
    }
    else
    {
        qDebug()<<"Not enough trackpoints to calculate duration, distance and speed";
        if(m_points.size() > 0)
        {
            QDateTime firstTime(m_points.at(0).time);
            m_time = firstTime.toLocalTime();
            emit timeChanged();
        }
    }
    emit trackChanged();
}

void TrackLoader::setFilename(const QString &filename) {
    if((m_filename == filename)) {
        qDebug()<<"No change in filename";
        return;
    }
    qDebug()<<"Setting filename"<<filename;
    m_filename = filename;
    emit filenameChanged();
    // Trigger loading
    m_loaded = false;
    m_error = false;
    load();
}

QString TrackLoader::filename() const
{
    return m_filename;
}

QString TrackLoader::name() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }
    return m_name;
}

QString TrackLoader::workout() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }
    return m_workout;
}

QString TrackLoader::description() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }
    return m_description;
}

QDateTime TrackLoader::time() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QDateTime();
    }
    return m_time;
}

QString TrackLoader::timeStr() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }
    if(!m_time.isValid()) {
        return QString();
    }
    return m_time.toString(Qt::SystemLocaleShortDate);
}

uint TrackLoader::duration()
{
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_duration;
}

uint TrackLoader::pauseDuration()
{
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_pause_duration;
}

qreal TrackLoader::elevationUp()
{
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_elevationUp;
}

qreal TrackLoader::elevationDown()
{
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_elevationDown;
}

QString TrackLoader::paceStr()
{
    if(!m_loaded && !m_error)
    {
        load();
    }
    if(!m_loaded || m_error)
    {
        // Nothing to load or error in loading
        return QString();
    }

    QString strPace = "";

    qreal rMinutes = qFloor(m_pace);
    qreal rSeconds = qCeil((m_pace * 60) - (rMinutes * 60));

    strPace = QString::number(rMinutes) + ":" + QString::number(rSeconds);

    return strPace;
}

QString TrackLoader::paceImperialStr()
{
    if(!m_loaded && !m_error)
    {
        load();
    }
    if(!m_loaded || m_error)
    {
        // Nothing to load or error in loading
        return QString();
    }

    qreal m_pace_imperial = m_pace * 1.609344;

    QString strPace = "";

    qreal rMinutes = qFloor(m_pace_imperial);
    qreal rSeconds = qCeil((m_pace_imperial * 60) - (rMinutes * 60));

    strPace = QString::number(rMinutes) + ":" + QString::number(rSeconds);

    return strPace;
}

QString TrackLoader::durationStr() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }

    uint hours = m_duration / (60*60);
    uint minutes = (m_duration - hours*60*60) / 60;
    uint seconds = m_duration - hours*60*60 - minutes*60;
    return TimeFormatter::formatHMS(hours, minutes, seconds);
}

QString TrackLoader::pauseDurationStr() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return QString();
    }

    //If there is no pause, return
    if (this->m_pause_positions.count() == 0)
        return QStringLiteral("-");

    uint hours = m_pause_duration / (60*60);
    uint minutes = (m_pause_duration - hours*60*60) / 60;
    uint seconds = m_pause_duration - hours*60*60 - minutes*60;

    return QString("%1/%2").arg(m_pause_positions.count()).arg(TimeFormatter::formatHMS(hours, minutes, seconds));
}

qreal TrackLoader::distance() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_distance;
}

qreal TrackLoader::speed() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_speed;
}

qreal TrackLoader::maxSpeed() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_maxSpeed;
}

qreal TrackLoader::pace() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_pace;
}

qreal TrackLoader::heartRate() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_heartRate;
}

uint TrackLoader::heartRateMin() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_heartRateMin;
}

uint TrackLoader::heartRateMax() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_heartRateMax;
}

bool TrackLoader::loaded() {
    return m_loaded;
}

int TrackLoader::pausePositionsCount()
{
    if(!m_loaded && !m_error)
    {
        load();
    }
    if(!m_loaded || m_error)
    {
        // Nothing to load or error in loading
        return 0;
    }
    return m_pause_positions.size();
}

int TrackLoader::pausePositionAt(int index)
{
    return m_pause_positions[index];
}

int TrackLoader::trackPointCount() {
    if(!m_loaded && !m_error) {
        load();
    }
    if(!m_loaded || m_error) {
        // Nothing to load or error in loading
        return 0;
    }
    return m_points.size();
}

QGeoCoordinate TrackLoader::trackPointAt(int index)
{
    return QGeoCoordinate(m_points.at(index).latitude,
                          m_points.at(index).longitude,
                          m_points.at(index).elevation);
}

uint TrackLoader::heartRateAt(int index)
{
    return m_points.at(index).heartrate;
}

qreal TrackLoader::elevationAt(int index)
{
    return m_points.at(index).elevation;
}

QDateTime TrackLoader::timeAt(int index)
{
    return m_points.at(index).time;
}

qint64 TrackLoader::unixTimeAt(int index)
{
    return m_points.at(index).time.toMSecsSinceEpoch() / 1000;
}

qreal TrackLoader::durationAt(int index)
{
    return m_points.at(index).duration;
}

qreal TrackLoader::distanceAt(int index)
{
    return m_points.at(index).distance;
}

qreal TrackLoader::speedAt(int index)
{
    return m_points.at(index).speed;
}

qreal TrackLoader::paceAt(int index)
{
    return m_points.at(index).pace;
}

QString TrackLoader::paceStrAt(int index)
{
    return paceToStr(m_points.at(index).pace);
}

QString TrackLoader::paceImperialStrAt(int index)
{
    return paceToStr(m_points.at(index).pace * 1.609344);
}

QString TrackLoader::paceToStr(qreal pace)
{
    int minutes = qFloor(pace);
    int seconds = qCeil((pace * 60) - (minutes * 60));

    return QString("%1:%2")
            .arg(minutes)
            .arg(seconds, 2, 10, QLatin1Char('0'));
}

int TrackLoader::fitZoomLevel(int width, int height) {
    if(m_points.size() < 2 || width < 1 || height < 1) {
        // One point track or zero size map
        return 20;
    }
    qreal minLat, maxLat, minLon, maxLon;
    minLat = maxLat = m_points.at(0).latitude;
    minLon = maxLon = m_points.at(0).longitude;
    for(int i=1;i<m_points.size();i++) {
        if(m_points.at(i).latitude < minLat) {
            minLat = m_points.at(i).latitude;
        } else if(m_points.at(i).latitude > maxLat) {
            maxLat = m_points.at(i).latitude;
        }
        if(m_points.at(i).longitude < minLon) {
            minLon = m_points.at(i).longitude;
        } else if(m_points.at(i).longitude > maxLon) {
            maxLon = m_points.at(i).longitude;
        }
    }

    m_center = QGeoCoordinate((minLat+maxLat)/2, (minLon+maxLon)/2);
    qreal coord, pixel;
    qreal trackMinX = (minLon + 180) / 360;
    qreal trackMaxX = (maxLon + 180) / 360;
    qreal trackMinY = sqrt(1-qLn(minLat*M_PI/180 + 1/qCos(minLat*M_PI/180))/M_PI);
    qreal trackMaxY = sqrt(1-qLn(maxLat*M_PI/180 + 1/qCos(maxLat*M_PI/180))/M_PI);

    qreal trackAR = qAbs((trackMaxX - trackMinX) / (trackMaxY - trackMinY));
    qreal windowAR = (qreal)width/(qreal)height;
    if(trackAR > windowAR ) {
        // Width limits
        coord = qAbs(trackMaxX - trackMinX);
        pixel = width;
    } else {
        // height limits
        coord = qAbs(trackMaxY - trackMinY);
        pixel = height;
    }

    // log2(x) = ln(x)/ln(2)
    int z = qFloor(qLn(pixel/256.0 * 1.0/coord * qCos(m_center.latitude()*M_PI/180))
                   / qLn(2)) + 1;
    return z;
}

QGeoCoordinate TrackLoader::center() {
    return m_center;
}
