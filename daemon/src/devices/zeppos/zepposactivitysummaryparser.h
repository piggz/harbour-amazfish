#ifndef ZEPPOSACTIVITYSUMMARYPARSER_H
#define ZEPPOSACTIVITYSUMMARYPARSER_H

#include <QByteArray>

#include <abstractactivitysummaryparser.h>

class ZeppOsActivitySummaryParser : public AbstractActivitySummaryParser
{
public:
    ZeppOsActivitySummaryParser();

    ActivitySummary parseBinaryData(const QByteArray& data, const QDateTime &starTime ) override;


private:
    struct Location {
        // TODO 2, 3
        uint32_t startTimestamp; // 1 unix epoch, seconds
        int32_t baseLatitude;    // 5 /6000000 -> coords
        int32_t baseLongitude;   // 6 /-6000000 -> coords
        int32_t baseAltitude;    // 7 /2 -> meters
        int32_t maxLatitude;     // 8 /3000000 -> coords
        int32_t minLatitude;     // 9 /3000000 -> coords
        int32_t maxLongitude;    // 10 /3000000 -> coords
        int32_t minLongitude;    // 11 /3000000 -> coords
    };

    struct HeartRate {
        int32_t avg; // 1 bpm
        int32_t max; // 2 bpm
        int32_t min; // 3 bpm
    };

    struct Steps {
        float avgCadence;  // 1 steps/sec
        float maxCadence;  // 2 steps/sec
        int32_t avgStride; // 3 cm
        int32_t steps;     // 4 count
    };

    struct Type {
        int32_t type; // 1  1 = running, 4 = bike, 3 = walk
        int32_t ai; // 2 0 = normal, 1 = ai/automatic
    };

    struct Distance {
        float distance; // 1 meters
    };

    struct Time {
        int32_t totalDuration; // 1 seconds
        int32_t workoutDuration; // 2 seconds
        int32_t pauseDuration; // 3 seconds
    };

    struct Pace {
        float avg; // 1 val * 1000 / 60 -> min/km
        float best; // 2 val * 1000 / 60 -> min/km
    };

    struct Altitude {
        int32_t maxAltitude = 1; // /200 -> meters
        int32_t minAltitude = 2; // /200 -> meters
        int32_t avgAltitude = 3; // /200 -> meters
        int32_t totalClimbing = 4; // cm
        int32_t elevationGain = 5; // cm
        int32_t elevationLoss = 6; // cm
    };

    struct Calories {
        int32_t calories = 1; // kcal
    };

    struct HeartRateZones {
        // TODO 1, is always = 1?
        // Zones: N/A, Warm-up, Fat-burn time, Aerobic, Anaerobic, Extreme
        std::vector<int32_t> zoneMax2; // 2 bpm
        std::vector<int32_t> zoneTime; // 1 seconds
    };

    struct Elevation {
        // TODO 1, 2?
        uint32_t uphillTime;   // 3 sec
        uint32_t downhillTime; // 4 sec
    };

    struct TrainingEffect {
        float aerobicTrainingEffect;   // 4
        float anaerobicTrainingEffect; // 5
        int32_t currentWorkoutLoad;    // 6
        int32_t maximumOxygenUptake;   // 7 ml/kg/min
    };

    struct SwimmingData {
        uint32_t style;          // 1 1: breaststroke, 2: freestyle
        uint32_t laps;           // 2
        uint32_t strokes;        // 3
        uint32_t avgDps;         // 4 cm
        uint32_t swolf;          // 5
        uint32_t avgStrokeRate;  // 6  // stroke/min
        uint32_t maxStrokeRate;  // 7 stroke/min
        uint32_t laneLength;     // 8
        uint32_t laneLengthUnit; //  9 0: meter, 1: yard
    };

    struct WorkoutSummary {
        std::string version;           // 1
        Location location;             // 2
        Type type;                     // 3
        Distance distance;             // 4
        Time time;                     // 7
        SwimmingData swimmingData;     // 9
        Pace pace;                     // 10
        Steps steps;                   // 11
        Altitude altitude;             // 13
        Calories calories;             // 16
        HeartRate heartRate;           // 19
        TrainingEffect trainingEffect; // 21
        HeartRateZones heartRateZones; // 22
        Elevation elevation;           // 23
    };

    WorkoutSummary m_summary;
};

#endif // ZEPPOSACTIVITYSUMMARYPARSER_H
