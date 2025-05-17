#include "zepposactivitysummaryparser.h"

#include <QDebug>
#include "picoproto.h"

ZeppOsActivitySummaryParser::ZeppOsActivitySummaryParser()
{

}

ActivitySummary ZeppOsActivitySummaryParser::parseBinaryData(const QByteArray &data, const QDateTime &startTime)
{
    qDebug() << Q_FUNC_INFO;

    picoproto::Message message;

    int version = (data[0] & 0xff) | ((data[1] & 0xff) << 8);
    if (version != 0x8000) {
        qDebug() << "Unexpected binary data version {}, parsing might fail: " << version;
    }

    ActivitySummary summary;

    if (!message.ParseFromBytes((uint8_t*)data.mid(2).data(), data.size() - 2)) {
        summary.setValid(false);
        return summary;
    }

    m_summary.version = message.GetString(1);

    //Location
    m_summary.location.startTimestamp = message.GetMessage(2)->GetUInt32(1);
    m_summary.location.baseLatitude = message.GetMessage(2)->GetInt32(5);
    m_summary.location.baseLongitude = message.GetMessage(2)->GetInt32(6);
    m_summary.location.baseAltitude = message.GetMessage(2)->GetInt32(7);
    m_summary.location.maxLatitude = message.GetMessage(2)->GetInt32(8);
    m_summary.location.minLatitude = message.GetMessage(2)->GetInt32(9);
    m_summary.location.maxLongitude = message.GetMessage(2)->GetInt32(10);
    m_summary.location.minLongitude = message.GetMessage(2)->GetInt32(11);

    //Type
    m_summary.type.type =message.GetMessage(3)->GetUInt32(1);
    m_summary.type.ai =message.GetMessage(3)->GetUInt32(2);

    //Distance
    m_summary.distance.distance = message.GetMessage(4)->GetFloat(1);

    //Time
    m_summary.time.totalDuration = message.GetMessage(7)->GetUInt32(1);
    m_summary.time.workoutDuration = message.GetMessage(7)->GetUInt32(2);
    //m_summary.time.pauseDuration = message.GetMessage(7)->GetInt32(3);

    //Swimming
    if (message.GetField(9)) {
        m_summary.swimmingData.style = message.GetMessage(9)->GetUInt32(1);
        m_summary.swimmingData.laps = message.GetMessage(9)->GetUInt32(2);
        m_summary.swimmingData.strokes = message.GetMessage(9)->GetUInt32(3);
        m_summary.swimmingData.avgDps = message.GetMessage(9)->GetUInt32(4);
        m_summary.swimmingData.swolf = message.GetMessage(9)->GetUInt32(5);
        m_summary.swimmingData.avgStrokeRate = message.GetMessage(9)->GetUInt32(6);
        m_summary.swimmingData.maxStrokeRate = message.GetMessage(9)->GetUInt32(7);
        m_summary.swimmingData.laneLength = message.GetMessage(9)->GetUInt32(8);
        m_summary.swimmingData.laneLengthUnit = message.GetMessage(9)->GetUInt32(9);
    }
    //Pace
    m_summary.pace.avg = message.GetMessage(10)->GetFloat(1);
    m_summary.pace.best = message.GetMessage(10)->GetFloat(2);

    //Steps
    m_summary.steps.avgCadence = message.GetMessage(11)->GetFloat(1);
    m_summary.steps.maxCadence = message.GetMessage(11)->GetFloat(2);
    m_summary.steps.avgStride = message.GetMessage(11)->GetInt32(3);
    m_summary.steps.steps = message.GetMessage(11)->GetInt32(4);

    //Altitiude
    m_summary.altitude.maxAltitude = message.GetMessage(13)->GetInt32(1);
    m_summary.altitude.minAltitude = message.GetMessage(13)->GetInt32(2);
    m_summary.altitude.avgAltitude = message.GetMessage(13)->GetInt32(3);
    m_summary.altitude.totalClimbing = message.GetMessage(13)->GetInt32(4);
    m_summary.altitude.elevationGain = message.GetMessage(13)->GetInt32(5);
    m_summary.altitude.elevationLoss = message.GetMessage(13)->GetInt32(6);

    //Calories
    m_summary.calories.calories = message.GetMessage(16)->GetInt32(1);

    //Heartrate
    m_summary.heartRate.avg = message.GetMessage(19)->GetInt32(1);
    m_summary.heartRate.max = message.GetMessage(19)->GetInt32(2);
    m_summary.heartRate.min = message.GetMessage(19)->GetInt32(3);

    //TrainigEffect
    m_summary.trainingEffect.aerobicTrainingEffect = message.GetMessage(21)->GetFloat(4);
    m_summary.trainingEffect.anaerobicTrainingEffect = message.GetMessage(21)->GetFloat(5);
    m_summary.trainingEffect.currentWorkoutLoad = message.GetMessage(21)->GetInt32(6);
    //m_summary.trainingEffect.aerobicTrainingEffect = message.GetMessage(21)->GetInt32(4);

    //HeartrateZones
    m_summary.heartRateZones.zoneMax2 = message.GetMessage(22)->GetInt32Array(2);
    m_summary.heartRateZones.zoneTime = message.GetMessage(22)->GetInt32Array(3);

    //Elevation
    m_summary.elevation.uphillTime = message.GetMessage(23)->GetUInt32(1);
    m_summary.elevation.downhillTime = message.GetMessage(23)->GetUInt32(2);

    ActivityKind::Type activityKind = ActivityKind::fromBipType(ActivityKind::Type(m_summary.type.type));

    qDebug() << "Raw activity data:" << m_summary.type.type << m_summary.time.totalDuration << m_summary.location.baseLatitude << m_summary.location.baseLongitude << m_summary.location.baseAltitude;
    summary.setStartTime(startTime);
    summary.setEndTime(startTime.addSecs(m_summary.time.totalDuration));
    summary.setActivityKind(activityKind);
    summary.setBaseLatitude(m_summary.location.baseLatitude / 6000000.0);
    summary.setBaseLongitude(m_summary.location.baseLongitude / -6000000.0);
    summary.setBaseAltitude(m_summary.location.baseAltitude / 2);

    summary.setName((ActivityKind::toString(summary.activityKind())) + "-" + summary.startTime().toLocalTime().toString("yyyyMMdd-HHmm"));
    summary.setValid(true);

    qDebug() << "Summary:" << summary.startTime() << summary.activityKind() << summary.baseLatitude() << summary.baseLongitude() << summary.baseAltitude();

    return summary;
}
