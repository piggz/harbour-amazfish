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
    if (message.GetField(2)) {
        m_summary.location.hasData = true;
        m_summary.location.startTimestamp = message.GetMessage(2)->GetUInt32(1);
        m_summary.location.baseLatitude = message.GetMessage(2)->GetInt32(5);
        m_summary.location.baseLongitude = message.GetMessage(2)->GetInt32(6);
        m_summary.location.baseAltitude = message.GetMessage(2)->GetInt32(7);
        m_summary.location.maxLatitude = message.GetMessage(2)->GetInt32(8);
        m_summary.location.minLatitude = message.GetMessage(2)->GetInt32(9);
        m_summary.location.maxLongitude = message.GetMessage(2)->GetInt32(10);
        m_summary.location.minLongitude = message.GetMessage(2)->GetInt32(11);
    }

    //Type
    if (message.GetField(3)) {
        m_summary.type.hasData = true;
        m_summary.type.type =message.GetMessage(3)->GetUInt32(1);
        m_summary.type.ai =message.GetMessage(3)->GetUInt32(2);
    }

    //Distance
    if (message.GetField(4)) {
        m_summary.distance.hasData = true;
        m_summary.distance.distance = message.GetMessage(4)->GetFloat(1);
    }

    //Time
    if (message.GetField(7)) {
        m_summary.time.hasData = true;
        m_summary.time.totalDuration = message.GetMessage(7)->GetUInt32(1);
        m_summary.time.workoutDuration = message.GetMessage(7)->GetUInt32(2);
        m_summary.time.pauseDuration = message.GetMessage(7)->GetInt32(3);
    }

    //Swimming
    if (message.GetField(9)) {
        m_summary.swimmingData.hasData = true;
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
    if (message.GetField(10)) {
        m_summary.pace.hasData = true;
        m_summary.pace.avg = message.GetMessage(10)->GetFloat(1);
        m_summary.pace.best = message.GetMessage(10)->GetFloat(2);
    }

    //Steps
    if (message.GetField(11)) {
        m_summary.steps.hasData = true;
        m_summary.steps.avgCadence = message.GetMessage(11)->GetFloat(1);
        m_summary.steps.maxCadence = message.GetMessage(11)->GetFloat(2);
        m_summary.steps.avgStride = message.GetMessage(11)->GetInt32(3);
        m_summary.steps.steps = message.GetMessage(11)->GetInt32(4);
    }

    //Altitiude
    if (message.GetField(13)) {
        m_summary.altitude.hasData = true;
        m_summary.altitude.maxAltitude = message.GetMessage(13)->GetInt32(1);
        m_summary.altitude.minAltitude = message.GetMessage(13)->GetInt32(2);
        m_summary.altitude.avgAltitude = message.GetMessage(13)->GetInt32(3);
        m_summary.altitude.totalClimbing = message.GetMessage(13)->GetInt32(4);
        m_summary.altitude.elevationGain = message.GetMessage(13)->GetInt32(5);
        m_summary.altitude.elevationLoss = message.GetMessage(13)->GetInt32(6);
    }

    //Calories
    if (message.GetField(16)) {
        m_summary.calories.hasData = true;
        m_summary.calories.calories = message.GetMessage(16)->GetInt32(1);
    }

    //Heartrate
    if (message.GetField(19)) {
        m_summary.heartRate.hasData = true;
        m_summary.heartRate.avg = message.GetMessage(19)->GetInt32(1);
        m_summary.heartRate.max = message.GetMessage(19)->GetInt32(2);
        m_summary.heartRate.min = message.GetMessage(19)->GetInt32(3);
    }

    //TrainigEffect
    if (message.GetField(21)) {
        m_summary.trainingEffect.hasData = true;
        m_summary.trainingEffect.aerobicTrainingEffect = message.GetMessage(21)->GetFloat(4);
        m_summary.trainingEffect.anaerobicTrainingEffect = message.GetMessage(21)->GetFloat(5);
        m_summary.trainingEffect.currentWorkoutLoad = message.GetMessage(21)->GetInt32(6);
        m_summary.trainingEffect.aerobicTrainingEffect = message.GetMessage(21)->GetInt32(4);

    }
    //HeartrateZones
    if (message.GetField(22)) {
        m_summary.heartRateZones.hasData = true;
        m_summary.heartRateZones.zoneMax2 = message.GetMessage(22)->GetInt32Array(2);
        m_summary.heartRateZones.zoneTime = message.GetMessage(22)->GetInt32Array(3);
    }

    //Elevation
    if (message.GetField(23)) {
        m_summary.elevation.hasData = true;
        m_summary.elevation.uphillTime = message.GetMessage(23)->GetUInt32(1);
        m_summary.elevation.downhillTime = message.GetMessage(23)->GetUInt32(2);
    }

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

    if (m_summary.location.hasData) {
        summary.addMetaData("maxLatitude", QString::number(convertHuamiValueToDecimalDegrees(m_summary.location.maxLatitude)), "deg");
        summary.addMetaData("maxLongitude", QString::number(convertHuamiValueToDecimalDegrees(m_summary.location.maxLongitude)), "deg");
        summary.addMetaData("minLatitude", QString::number(convertHuamiValueToDecimalDegrees(m_summary.location.minLatitude)), "deg");
        summary.addMetaData("minLongitude", QString::number(convertHuamiValueToDecimalDegrees(m_summary.location.minLongitude)), "deg");
    }

    if (m_summary.distance.hasData) {
        summary.addMetaData("distanceMeters", QString::number(m_summary.distance.distance), "meters");
    }

    if (m_summary.time.hasData) {
        summary.addMetaData("totalDuration", QString::number(m_summary.time.totalDuration), "seconds");
        summary.addMetaData("workoutDuration", QString::number(m_summary.time.workoutDuration), "seconds");
        summary.addMetaData("pauseDuration", QString::number(m_summary.time.pauseDuration), "seconds");
    }

    if (m_summary.swimmingData.hasData) {
        summary.addMetaData("swimLaps", QString::number(m_summary.swimmingData.laps), "laps");
        summary.addMetaData("swimStrokes", QString::number(m_summary.swimmingData.strokes), "strokes");
        summary.addMetaData("swimAvgDps", QString::number(m_summary.swimmingData.avgDps), "");
        summary.addMetaData("swolfIndex", QString::number(m_summary.swimmingData.laps), "swolf_index");
        summary.addMetaData("swimAvgStrokeRate", QString::number(m_summary.swimmingData.avgStrokeRate), "strokes_second");
        summary.addMetaData("swimAvgMaxRate", QString::number(m_summary.swimmingData.maxStrokeRate), "strokes_second");
        summary.addMetaData("swimLaneLength", QString::number(m_summary.swimmingData.laneLength), "meers");
    }

    if (m_summary.pace.hasData) {
        summary.addMetaData("paceAvg", QString::number(m_summary.pace.avg * 1000 / 60), "min_km");
        summary.addMetaData("paceBest", QString::number(m_summary.pace.best * 1000 / 60), "min_km");
    }

    if (m_summary.steps.hasData) {
        summary.addMetaData("stepsAvgCadence", QString::number(m_summary.pace.avg), "steps_min");
        summary.addMetaData("stepsMaxCadence", QString::number(m_summary.pace.best), "steps_min");
        summary.addMetaData("stepsAvgStride", QString::number(m_summary.pace.avg), "cm");
        summary.addMetaData("steps", QString::number(m_summary.steps.steps), "steps");
    }

    if (m_summary.altitude.hasData) {
        summary.addMetaData("maxAltitude", QString::number(m_summary.altitude.maxAltitude / 200), "meters");
        summary.addMetaData("minAltitude", QString::number(m_summary.altitude.minAltitude / 200), "meters");
        summary.addMetaData("avgAltitude", QString::number(m_summary.altitude.avgAltitude / 200), "meters");
        summary.addMetaData("totalClimbing", QString::number(m_summary.altitude.totalClimbing / 100), "meters");
        summary.addMetaData("elevationGain", QString::number(m_summary.altitude.elevationGain / 100), "meters");
        summary.addMetaData("elevationLoss", QString::number(m_summary.altitude.elevationLoss / 100), "meters");
    }

    if (m_summary.calories.hasData) {
        summary.addMetaData("calories", QString::number(m_summary.calories.calories), "calories");
    }

    if (m_summary.heartRate.hasData) {
        summary.addMetaData("avgHeartRate", QString::number(m_summary.heartRate.avg), "bpm");
        summary.addMetaData("maxHeartRate", QString::number(m_summary.heartRate.max), "bpm");
        summary.addMetaData("minHeartRate", QString::number(m_summary.heartRate.min), "bpm");
    }

    if (m_summary.trainingEffect.hasData) {
        summary.addMetaData("aerobicTrainingEffect", QString::number(m_summary.trainingEffect.aerobicTrainingEffect), "");
        summary.addMetaData("anaerobicTrainingEffect", QString::number(m_summary.trainingEffect.anaerobicTrainingEffect), "");
        summary.addMetaData("currentWorkoutLoad", QString::number(m_summary.trainingEffect.currentWorkoutLoad), "");
        summary.addMetaData("maximumOxygenUptake", QString::number(m_summary.trainingEffect.maximumOxygenUptake), "ml_kg_min");
    }

    if (m_summary.elevation.hasData) {
        summary.addMetaData("uphillTime", QString::number(m_summary.elevation.uphillTime), "seconds");
        summary.addMetaData("downhillTime", QString::number(m_summary.elevation.downhillTime), "seconds");
    }

    qDebug() << "Summary:" << summary.startTime() << summary.activityKind() << summary.baseLatitude() << summary.baseLongitude() << summary.baseAltitude();

    return summary;
}
