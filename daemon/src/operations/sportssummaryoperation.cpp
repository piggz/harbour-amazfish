#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"

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

SportsSummaryOperation::SportsSummaryOperation(QBLEService *service, KDbConnection *conn) : AbstractFetchOperation(service)
{
    m_conn = conn;
    setLastSyncKey("device/lastsportsyncmillis");
}

void SportsSummaryOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, false);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES) + rawDate);
}

void SportsSummaryOperation::handleData(const QByteArray &data)
{
    if (data.length() < 2) {
        qDebug() << Q_FUNC_INFO << "unexpected sports summary data length: " << data.length();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Data counter:" << data[0];
    if ((m_lastPacketCounter + 1) == data[0] ) {
        m_lastPacketCounter++;
        if (m_lastPacketCounter > 255) {
            m_lastPacketCounter = 0;
        }
        m_buffer += data.mid(1);
    } else {
        qDebug() << Q_FUNC_INFO << "invalid package counter: " << data[0] << ", last was: " << m_lastPacketCounter;
        finished(false);
        return;
    }
}

bool SportsSummaryOperation::finished(bool success)
{
    if (success) {
        parseSummary();
        m_summary.setName(activityName());
        m_success = true;
    }
    return success;
}

void SportsSummaryOperation::parseSummary()
{
    qDebug() << Q_FUNC_INFO << "Buffer:" << m_buffer.toHex();

    summary_t summaryData;
    memcpy(&summaryData, m_buffer.data(), sizeof(summary_t));

    ActivityKind::Type activityKind = ActivityKind::fromBipType(ActivityKind::Type(summaryData.kind));
    qDebug() << Q_FUNC_INFO << summaryData.version << summaryData.kind << summaryData.timestamp_start << summaryData.timestamp_end;

    long duration = summaryData.timestamp_end - summaryData.timestamp_start;
    m_summary.setStartTime(startDate());
    m_summary.setEndTime(startDate().addSecs(duration));
    m_summary.setActivityKind(activityKind);
    m_summary.setBaseLongitude(summaryData.baseLongitude);
    m_summary.setBaseLatitude(summaryData.baseLatitude);
    m_summary.setBaseAltitude(summaryData.baseAltitude);

    if (summaryData.version >= 512) {
        m_summary.addMetaData("steps", QString::number(summaryData.format512.steps), "steps_unit");
        m_summary.addMetaData("activeSeconds", QString::number(summaryData.format512.activeSeconds), "seconds");
        m_summary.addMetaData("caloriesBurnt", QString::number(summaryData.format512.caloriesBurnt), "calories_unit");
        m_summary.addMetaData("distanceMeters", QString::number(summaryData.format512.distanceMeters), "meters");
        m_summary.addMetaData("ascentMeters", QString::number(summaryData.format512.ascentMeters), "meters");
        m_summary.addMetaData("descentMeters", QString::number(summaryData.format512.descentMeters), "meters");
        if (summaryData.format512.maxAltitude != -100000) {
            m_summary.addMetaData("maxAltitude", QString::number(summaryData.format512.maxAltitude), "meters");
        }
        if (summaryData.format512.minAltitude != 100000) {
            m_summary.addMetaData("minAltitude", QString::number(summaryData.format512.minAltitude), "meters");
        }

        if (!(activityKind == ActivityKind::EllipticalTrainer ||
                activityKind == ActivityKind::JumpRope ||
                activityKind == ActivityKind::Exercise ||
                activityKind == ActivityKind::Yoga ||
                activityKind == ActivityKind::IndoorCycling)) {
            m_summary.addMetaData("minPace", QString::number(summaryData.format512.minPace), "seconds_m");
            m_summary.addMetaData("maxPace", QString::number(summaryData.format512.maxPace), "seconds_m");
        }

        m_summary.addMetaData("averageHR", QString::number(summaryData.format512.averageHR), "bpm");
        m_summary.addMetaData("maxHR", QString::number(summaryData.format512.maxHR), "bpm");
        m_summary.addMetaData("averageKMPaceSeconds", QString::number(summaryData.format512.averageKMPaceSeconds), "seconds_km");
        m_summary.addMetaData("averageStride", QString::number(summaryData.format512.averageStride), "cm");

        if (summaryData.kind == ActivityKind::Cycling || summaryData.kind == ActivityKind::Running) {
            m_summary.addMetaData("ascentSeconds", QString::number(summaryData.format512.runcycle.ascentSeconds / 1000), "seconds");
            m_summary.addMetaData("descentSeconds", QString::number(summaryData.format512.runcycle.descentSeconds / 1000), "seconds");
            m_summary.addMetaData("flatSeconds", QString::number(summaryData.format512.runcycle.flatSeconds / 1000), "seconds");
        } else if (activityKind == ActivityKind::Swimming || activityKind == ActivityKind::OpenSwimming) {
            m_summary.addMetaData("averageStrokeDistance", QString::number(summaryData.format512.swimming.averageStrokeDistance), "meters");
            m_summary.addMetaData("averageStrokesPerSecond", QString::number(summaryData.format512.swimming.averageStrokesPerSecond), "strokes_second");
            m_summary.addMetaData("averageLapPace", QString::number(summaryData.format512.swimming.averageLapPace), "seconds");
            m_summary.addMetaData("strokes", QString::number(summaryData.format512.swimming.strokes), "strokes");
            m_summary.addMetaData("swolfIndex", QString::number(summaryData.format512.swimming.swolfIndex), "swolf_index");
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
            m_summary.addMetaData("swimStyle", swimStyleName, "style");
            m_summary.addMetaData("laps", QString::number(summaryData.format512.swimming.laps), "laps");
        }
    } else {
        m_summary.addMetaData("distanceMeters", QString::number(summaryData.format256.distanceMeters), "meters");
        m_summary.addMetaData("ascentMeters", QString::number(summaryData.format256.ascentMeters), "meters");
        m_summary.addMetaData("descentMeters", QString::number(summaryData.format256.descentMeters), "meters");
        m_summary.addMetaData("minAltitude", QString::number(summaryData.format256.minAltitude), "meters");
        m_summary.addMetaData("maxAltitude", QString::number(summaryData.format256.maxAltitude), "meters");
        m_summary.addMetaData("maxLatitude", QString::number(summaryData.format256.minLatitude), "meters");
        m_summary.addMetaData("minLatitude", QString::number(summaryData.format256.maxLatitude), "meters");
        m_summary.addMetaData("maxLongitude", QString::number(summaryData.format256.minLongitude), "meters");
        m_summary.addMetaData("minLongitude", QString::number(summaryData.format256.maxLongitude), "meters");
        m_summary.addMetaData("steps", QString::number(summaryData.format256.steps), "steps_unit");
        m_summary.addMetaData("activeSeconds", QString::number(summaryData.format256.activeSeconds), "seconds");
        m_summary.addMetaData("caloriesBurnt", QString::number(summaryData.format256.caloriesBurnt), "calories_unit");
        m_summary.addMetaData("maxSpeed", QString::number(summaryData.format256.maxSpeed), "meters_second");
        m_summary.addMetaData("averageHR", QString::number(summaryData.format256.averageHR), "bpm");
        m_summary.addMetaData("averageKMPaceSeconds", QString::number(summaryData.format256.averageKMPaceSeconds), "seconds_km");
        m_summary.addMetaData("averageStride", QString::number(summaryData.format256.averageStride), "seconds_km");

        if (!(activityKind == ActivityKind::EllipticalTrainer ||
                activityKind == ActivityKind::JumpRope ||
                activityKind == ActivityKind::Exercise ||
                activityKind == ActivityKind::Yoga ||
                activityKind == ActivityKind::IndoorCycling)) {
            m_summary.addMetaData("minPace", QString::number(summaryData.format256.minPace), "seconds_m");
            m_summary.addMetaData("maxPace", QString::number(summaryData.format256.maxPace), "seconds_m");
        }
        m_summary.addMetaData("totalStride", QString::number(summaryData.format256.totalStride), "meters");

        if (summaryData.kind == ActivityKind::Cycling || summaryData.kind == ActivityKind::Running) {
            m_summary.addMetaData("ascentSeconds", QString::number(summaryData.format256.runcycle.ascentSeconds / 1000), "seconds");
            m_summary.addMetaData("descentSeconds", QString::number(summaryData.format256.runcycle.descentSeconds / 1000), "seconds");
            m_summary.addMetaData("flatSeconds", QString::number(summaryData.format256.runcycle.flatSeconds / 1000), "seconds");
        } else if (activityKind == ActivityKind::Swimming || activityKind == ActivityKind::OpenSwimming) {
            m_summary.addMetaData("averageStrokeDistance", QString::number(summaryData.format256.swimming.averageStrokeDistance), "meters");
            m_summary.addMetaData("averageStrokesPerSecond", QString::number(summaryData.format256.swimming.averageStrokesPerSecond), "strokes_second");
            m_summary.addMetaData("averageLapPace", QString::number(summaryData.format256.swimming.averageLapPace), "seconds");
            m_summary.addMetaData("strokes", QString::number(summaryData.format256.swimming.strokes), "strokes");
            m_summary.addMetaData("swolfIndex", QString::number(summaryData.format256.swimming.swolfIndex), "swolf_index");
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
            m_summary.addMetaData("swimStyle", swimStyleName, "style");
            m_summary.addMetaData("laps", QString::number(summaryData.format256.swimming.laps), "laps");
        }
    }
}

bool SportsSummaryOperation::success() const
{
    return m_success;
}

ActivitySummary SportsSummaryOperation::summary()
{
    return m_summary;
}

QString SportsSummaryOperation::activityName()
{
    qDebug() << Q_FUNC_INFO << "Getting activity name";
    return (ActivityKind::toString(m_summary.activityKind())) + "-" + m_summary.startTime().toLocalTime().toString("yyyyMMdd-HHmm");
}
