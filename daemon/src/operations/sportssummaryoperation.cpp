#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"

SportsSummaryOperation::SportsSummaryOperation(QBLEService *service, KDbConnection *conn) : AbstractFetchOperation(service)
{
    m_conn = conn;
    setLastSyncKey("device/lastsportsyncmillis");
}

void SportsSummaryOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    qDebug() << "last summary sync was" << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES) + rawDate);
}

void SportsSummaryOperation::handleData(const QByteArray &data)
{
    if (data.length() < 2) {
        qDebug() << "unexpected sports summary data length: " << data.length();
        return;
    }

    qDebug() << "Data counter:" << data[0];
    if ((m_lastPacketCounter + 1) == data[0] ) {
        m_lastPacketCounter++;
        if (m_lastPacketCounter > 255) {
            m_lastPacketCounter = 0;
        }
        m_buffer += data.mid(1);
    } else {
        qDebug() << "invalid package counter: " << data[0] << ", last was: " << m_lastPacketCounter;
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
    qDebug() << "Buffer:" << m_buffer.toHex();

    QDataStream stream(m_buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint16 version;
    stream >> version;
    m_summary.setVersion(version);

    quint16 kind;
    stream >> kind;

    ActivityKind::Type  activityKind = ActivityKind::Unknown;
    activityKind = ActivityKind::fromBipType(kind);
    m_summary.setActivityKind(activityKind);

    // FIXME: should honor timezone we were in at that time etc
    quint32 timestamp_start = 0;
    quint32 timestamp_end = 0;
    
    stream >> timestamp_start;
    stream >> timestamp_end;

    qDebug() << "Time:" << timestamp_start << timestamp_end << sizeof(int) << sizeof(short);

    // FIXME: should be done like this but seems to return crap when in DST
    //summary.setStartTime(new Date(timestamp_start));
    //summary.setEndTime(new Date(timestamp_end));

    // FIXME ... so do it like this
    long duration = timestamp_end - timestamp_start;
    //summary->setStartTime(new Date(getLastStartTimestamp().getTimeInMillis()));
    //summary->setEndTime(new Date(getLastStartTimestamp().getTimeInMillis() + duration));

    //startDate is in format that doent include DST, so we need to convert it to a real dt
    QDateTime temp = QDateTime::fromString(startDate().toString("yyyy-MM-dd hh:mm:ss"), "yyyy-MM-dd hh:mm:ss");
    QDateTime start = temp.toTimeSpec(Qt::UTC);
    qDebug() << "Time convertion:" << temp << start << startDate();

    m_summary.setStartTime(start);
    m_summary.setEndTime(start.addSecs(duration));


    qint32 baseLongitude = 0;
    qint32 baseLatitude = 0;
    qint32 baseAltitude = 0;
    stream >> baseLongitude >> baseLatitude >> baseAltitude;

    m_summary.setBaseLongitude(baseLongitude);
    m_summary.setBaseLatitude(baseLatitude);
    m_summary.setBaseAltitude(baseAltitude);
    

    //        summary.setBaseCoordinate(new GPSCoordinate(baseLatitude, baseLongitude, baseAltitude));

    //        summary.setDistanceMeters(Float.intBitsToFloat(buffer.getInt()));
    //        summary.setAscentMeters(Float.intBitsToFloat(buffer.getInt()));
    //        summary.setDescentMeters(Float.intBitsToFloat(buffer.getInt()));
    //
    //        summary.setMinAltitude(Float.intBitsToFloat(buffer.getInt()));
    //        summary.setMaxAltitude(Float.intBitsToFloat(buffer.getInt()));
    //        summary.setMinLatitude(buffer.getInt());
    //        summary.setMaxLatitude(buffer.getInt());
    //        summary.setMinLongitude(buffer.getInt());
    //        summary.setMaxLongitude(buffer.getInt());
    //
    //        summary.setSteps(BLETypeConversions.toUnsigned(buffer.getInt()));
    //        summary.setActiveTimeSeconds(BLETypeConversions.toUnsigned(buffer.getInt()));
    //
    //        summary.setCaloriesBurnt(Float.intBitsToFloat(buffer.get()));
    //        summary.setMaxSpeed(Float.intBitsToFloat(buffer.get()));
    //        summary.setMinPace(Float.intBitsToFloat(buffer.get()));
    //        summary.setMaxPace(Float.intBitsToFloat(buffer.get()));
    //        summary.setTotalStride(Float.intBitsToFloat(buffer.get()));

    //buffer.getInt(); //
    //buffer.getInt(); //
    //buffer.getInt(); //

    //        summary.setTimeAscent(BLETypeConversions.toUnsigned(buffer.getInt()));
    //        buffer.getInt(); //
    //        summary.setTimeDescent(BLETypeConversions.toUnsigned(buffer.getInt()));
    //        buffer.getInt(); //
    //        summary.setTimeFlat(BLETypeConversions.toUnsigned(buffer.getInt()));
    //
    //        summary.setAverageHR(BLETypeConversions.toUnsigned(buffer.getShort()));
    //
    //        summary.setAveragePace(BLETypeConversions.toUnsigned(buffer.getShort()));
    //        summary.setAverageStride(BLETypeConversions.toUnsigned(buffer.getShort()));

    //buffer.getShort(); //
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
    qDebug() << "Getting activity name";
    return (ActivityKind::toString(m_summary.activityKind())) + "-" + m_summary.startTime().toLocalTime().toString("yyyyMMdd-HHmm");
}
