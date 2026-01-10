#include "huamiactivitysummaryparser.h"

#pragma pack(push, 1)
struct summary_t {
    qint16 version;
    qint16 kind;
    quint32 timestamp_start;
    quint32 timestamp_end;
    qint32 baseLongitude;
    qint32 baseLatitude;
    qint32 baseAltitude;

    union {
        struct {
            qint32 steps;
            qint32 activeSeconds;
            qint32 unknown1;
            qint32 unknown2;
            qint32 unknown3;
            qint32 unknown4;
            float caloriesBurnt;
            float distanceMeters;
            float ascentMeters;
            float descentMeters;
            float maxAltitude;
            float minAltitude;
            qint32 unknown5;
            qint32 unknown6;
            qint32 unknown7;
            qint32 unknown8;
            float minPace;
            float maxPace;
            qint32 unknown9;
            qint32 unknown10;
            qint32 unknown11;
            qint32 unknown12;
            qint32 unknown13;
            qint32 unknown14;
            qint32 unknown15;
            qint32 unknown16;
            qint32 unknown17;
            qint16 averageHR;
            qint16 averageKMPaceSeconds;
            qint16 averageStride;
            qint16 maxHR;

            union {
                struct {
                    qint32 unknown1;
                    qint32 unknown2;
                    qint32 ascentSeconds;
                    qint32 unknown3;
                    qint32 descentSeconds;
                    qint32 unknown4;
                    qint32 flatSeconds;
                } runcycle;
                struct {
                    float averageStrokeDistance;
                    qint32 unknown1;
                    qint32 unknown2;
                    qint32 unknown3;
                    qint32 unknown4;
                    qint32 unknown5;
                    float averageStrokesPerSecond;
                    float averageLapPace;
                    qint16 strokes;
                    qint16 swolfIndex; // this is called SWOLF score on bip s, SWOLF index on mi band 4
                    qint8 swimStyle;
                    qint8 laps;
                    qint8 unknown6;
                    qint8 unknwonw7;
                } swimming;
            };
        } format512;
        struct {
            float distanceMeters;
            float ascentMeters;
            float descentMeters;
            float minAltitude;
            float maxAltitude;
            float maxLatitude;
            float minLatitude;
            float maxLongitude;
            float minLongitude;
            qint32 steps;
            qint32 activeSeconds;
            float caloriesBurnt;
            float maxSpeed;
            float maxPace;
            float minPace;
            float totalStride;
            qint32 unknown1;

            union {
                struct {
                    qint32 unknown1;
                    qint32 unknown2;
                    qint32 ascentSeconds;
                    qint32 unknown3;
                    qint32 descentSeconds;
                    qint32 unknown4;
                    qint32 flatSeconds;
                } runcycle;
                struct {
                    float averageStrokeDistance;
                    float averageStrokesPerSecond;
                    float averageLapPace;
                    qint16 strokes;
                    qint16 swolfIndex; // this is called SWOLF score on bip s, SWOLF index on mi band 4
                    qint8 swimStyle;
                    qint8 laps;
                    qint16 unknown1;
                    qint32 unknown2;
                    qint32 unknown3;
                } swimming;
            };
            qint16 averageHR;
            qint16 averageKMPaceSeconds;
            qint16 averageStride;
        } format256;
    };
};
#pragma pack(pop)

HuamiActivitySummaryParser::HuamiActivitySummaryParser()
{

}

ActivitySummary HuamiActivitySummaryParser::parseBinaryData(const QByteArray &data, const QDateTime &startTime)
{
    summary_t summaryData;
    ActivitySummary summary;

    memcpy(&summaryData, data.data(), sizeof(summary_t));

    ActivityKind::Type activityKind = ActivityKind::fromBipType(ActivityKind::Type(summaryData.kind));
    qDebug() << Q_FUNC_INFO << summaryData.version << summaryData.kind << summaryData.timestamp_start << summaryData.timestamp_end;

    long duration = summaryData.timestamp_end - summaryData.timestamp_start;
    summary.setStartTime(startTime);
    summary.setEndTime(startTime.addSecs(duration));
    summary.setActivityKind(activityKind);
    summary.setBaseLongitude(summaryData.baseLongitude / HUAMI_TO_DECIMAL_DEGREES_DIVISOR);
    summary.setBaseLatitude(summaryData.baseLatitude / HUAMI_TO_DECIMAL_DEGREES_DIVISOR);
    summary.setBaseAltitude(summaryData.baseAltitude);

    if (summaryData.version >= 512) {
        summary.addMetaData("steps", QString::number(summaryData.format512.steps), "steps_unit");
        summary.addMetaData("activeSeconds", QString::number(summaryData.format512.activeSeconds), "seconds");
        summary.addMetaData("caloriesBurnt", QString::number(summaryData.format512.caloriesBurnt), "calories_unit");
        summary.addMetaData("distanceMeters", QString::number(summaryData.format512.distanceMeters), "meters");
        summary.addMetaData("ascentMeters", QString::number(summaryData.format512.ascentMeters), "meters");
        summary.addMetaData("descentMeters", QString::number(summaryData.format512.descentMeters), "meters");
        if (summaryData.format512.maxAltitude != -100000) {
            summary.addMetaData("maxAltitude", QString::number(summaryData.format512.maxAltitude), "meters");
        }
        if (summaryData.format512.minAltitude != 100000) {
            summary.addMetaData("minAltitude", QString::number(summaryData.format512.minAltitude), "meters");
        }

        if (!(activityKind == ActivityKind::EllipticalTrainer ||
                activityKind == ActivityKind::JumpRope ||
                activityKind == ActivityKind::Exercise ||
                activityKind == ActivityKind::Yoga ||
                activityKind == ActivityKind::IndoorCycling)) {
            summary.addMetaData("minPace", QString::number(summaryData.format512.minPace), "seconds_m");
            summary.addMetaData("maxPace", QString::number(summaryData.format512.maxPace), "seconds_m");
        }

        summary.addMetaData("averageHR", QString::number(summaryData.format512.averageHR), "bpm");
        summary.addMetaData("maxHR", QString::number(summaryData.format512.maxHR), "bpm");
        summary.addMetaData("averageKMPaceSeconds", QString::number(summaryData.format512.averageKMPaceSeconds), "seconds_km");
        summary.addMetaData("averageStride", QString::number(summaryData.format512.averageStride), "cm");

        if (summaryData.kind == ActivityKind::Cycling || summaryData.kind == ActivityKind::Running) {
            summary.addMetaData("ascentSeconds", QString::number(summaryData.format512.runcycle.ascentSeconds / 1000), "seconds");
            summary.addMetaData("descentSeconds", QString::number(summaryData.format512.runcycle.descentSeconds / 1000), "seconds");
            summary.addMetaData("flatSeconds", QString::number(summaryData.format512.runcycle.flatSeconds / 1000), "seconds");
        } else if (activityKind == ActivityKind::Swimming || activityKind == ActivityKind::OpenSwimming) {
            summary.addMetaData("averageStrokeDistance", QString::number(summaryData.format512.swimming.averageStrokeDistance), "meters");
            summary.addMetaData("averageStrokesPerSecond", QString::number(summaryData.format512.swimming.averageStrokesPerSecond), "strokes_second");
            summary.addMetaData("averageLapPace", QString::number(summaryData.format512.swimming.averageLapPace), "seconds");
            summary.addMetaData("strokes", QString::number(summaryData.format512.swimming.strokes), "strokes");
            summary.addMetaData("swolfIndex", QString::number(summaryData.format512.swimming.swolfIndex), "swolf_index");
            QString swimStyleName = "unknown"; // TODO: translate here or keep as string identifier here?
            switch (summaryData.format512.swimming.swimStyle) {
                case 1:
                    swimStyleName = "breaststroke";
                    break;
                case 2:
                    swimStyleName = "freestyle";
                    break;
                case 3:
                    swimStyleName = "backstroke";
                    break;
                case 4:
                    swimStyleName = "medley";
                    break;
            }
            summary.addMetaData("swimStyle", swimStyleName, "style");
            summary.addMetaData("laps", QString::number(summaryData.format512.swimming.laps), "laps");
        }
    } else {
        summary.addMetaData("distanceMeters", QString::number(summaryData.format256.distanceMeters), "meters");
        summary.addMetaData("ascentMeters", QString::number(summaryData.format256.ascentMeters), "meters");
        summary.addMetaData("descentMeters", QString::number(summaryData.format256.descentMeters), "meters");
        summary.addMetaData("minAltitude", QString::number(summaryData.format256.minAltitude), "meters");
        summary.addMetaData("maxAltitude", QString::number(summaryData.format256.maxAltitude), "meters");
        summary.addMetaData("maxLatitude", QString::number(summaryData.format256.minLatitude), "meters");
        summary.addMetaData("minLatitude", QString::number(summaryData.format256.maxLatitude), "meters");
        summary.addMetaData("maxLongitude", QString::number(summaryData.format256.minLongitude), "meters");
        summary.addMetaData("minLongitude", QString::number(summaryData.format256.maxLongitude), "meters");
        summary.addMetaData("steps", QString::number(summaryData.format256.steps), "steps_unit");
        summary.addMetaData("activeSeconds", QString::number(summaryData.format256.activeSeconds), "seconds");
        summary.addMetaData("caloriesBurnt", QString::number(summaryData.format256.caloriesBurnt), "calories_unit");
        summary.addMetaData("maxSpeed", QString::number(summaryData.format256.maxSpeed), "meters_second");
        summary.addMetaData("averageHR", QString::number(summaryData.format256.averageHR), "bpm");
        summary.addMetaData("averageKMPaceSeconds", QString::number(summaryData.format256.averageKMPaceSeconds), "seconds_km");
        summary.addMetaData("averageStride", QString::number(summaryData.format256.averageStride), "seconds_km");

        if (!(activityKind == ActivityKind::EllipticalTrainer ||
                activityKind == ActivityKind::JumpRope ||
                activityKind == ActivityKind::Exercise ||
                activityKind == ActivityKind::Yoga ||
                activityKind == ActivityKind::IndoorCycling)) {
            summary.addMetaData("minPace", QString::number(summaryData.format256.minPace), "seconds_m");
            summary.addMetaData("maxPace", QString::number(summaryData.format256.maxPace), "seconds_m");
        }
        summary.addMetaData("totalStride", QString::number(summaryData.format256.totalStride), "meters");

        if (summaryData.kind == ActivityKind::Cycling || summaryData.kind == ActivityKind::Running) {
            summary.addMetaData("ascentSeconds", QString::number(summaryData.format256.runcycle.ascentSeconds / 1000), "seconds");
            summary.addMetaData("descentSeconds", QString::number(summaryData.format256.runcycle.descentSeconds / 1000), "seconds");
            summary.addMetaData("flatSeconds", QString::number(summaryData.format256.runcycle.flatSeconds / 1000), "seconds");
        } else if (activityKind == ActivityKind::Swimming || activityKind == ActivityKind::OpenSwimming) {
            summary.addMetaData("averageStrokeDistance", QString::number(summaryData.format256.swimming.averageStrokeDistance), "meters");
            summary.addMetaData("averageStrokesPerSecond", QString::number(summaryData.format256.swimming.averageStrokesPerSecond), "strokes_second");
            summary.addMetaData("averageLapPace", QString::number(summaryData.format256.swimming.averageLapPace), "seconds");
            summary.addMetaData("strokes", QString::number(summaryData.format256.swimming.strokes), "strokes");
            summary.addMetaData("swolfIndex", QString::number(summaryData.format256.swimming.swolfIndex), "swolf_index");
            QString swimStyleName = "unknown"; // TODO: translate here or keep as string identifier here?
            switch (summaryData.format512.swimming.swimStyle) {
                case 1:
                    swimStyleName = "breaststroke";
                    break;
                case 2:
                    swimStyleName = "freestyle";
                    break;
                case 3:
                    swimStyleName = "backstroke";
                    break;
                case 4:
                    swimStyleName = "medley";
                    break;
            }
            summary.addMetaData("swimStyle", swimStyleName, "style");
            summary.addMetaData("laps", QString::number(summaryData.format256.swimming.laps), "laps");
        }
    }
    summary.setName((ActivityKind::toString(summary.activityKind())) + "-" + summary.startTime().toLocalTime().toString("yyyyMMdd-HHmm"));

    summary.setValid(true);
    return summary;
}
