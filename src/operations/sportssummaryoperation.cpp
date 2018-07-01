#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"

SportsSummaryOperation::SportsSummaryOperation(QBLEService *service, KDbConnection *conn) : AbstractOperation(service)
{
    m_conn = conn;
}

void SportsSummaryOperation::start()
{
    m_startDate = lastActivitySync();


    qDebug() << "last summary sync was" << m_startDate;

    QByteArray rawDate = TypeConversion::dateTimeToBytes(m_startDate, 0);

    MiBandService *serv = dynamic_cast<MiBandService*>(m_service);

    m_service->enableNotification(serv->UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, serv->COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, serv->COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES) + rawDate);
    //Send log read command
    m_service->writeValue(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, serv->COMMAND_FETCH_DATA));
}

bool SportsSummaryOperation::handleMetaData(const QByteArray &value)
{
    if (value.length() == 15) {
        // first two bytes are whether our request was accepted
        if (value.mid(0, 3) == QByteArray(dynamic_cast<MiBandService*>(m_service)->RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS, 3)) {
            // the third byte (0x01 on success) = ?
            // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
            int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

            // last 8 bytes are the start date
            QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), false);
            setStartDate(startDate);

            qDebug() << "About to transfer activity data from " << startDate;
            m_service->message(QObject::tr("About to transfer activity data from ") + startDate.toString());

        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else if (value.length() == 3) {
        if (value == QByteArray(dynamic_cast<MiBandService*>(m_service)->RESPONSE_FINISH_SUCCESS, 3)) {
            qDebug() << "Finished sending data";
            finished(true);
            m_service->message(QObject::tr("Finished transferring activity data"));
            return true;
        } else if (value == QByteArray(dynamic_cast<MiBandService*>(m_service)-> RESPONSE_FINISH_FAIL, 3)) {
            qDebug() << "No data left to fetch";
            m_service->message(QObject::tr("No data to transfer"));
            return true;
        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else {
        qDebug() << "Unexpected activity metadata: " << value;
    }
    return false;
}



void SportsSummaryOperation::handleData(const QByteArray &data)
{
    if (data.length() < 2) {
        qDebug() << "unexpected sports summary data length: " << data.length();
        return;
    }

    if ((m_lastPacketCounter + 1) == data[0] ) {
        m_lastPacketCounter++;
        m_buffer += data.mid(1);
    } else {
        qDebug() << "invalid package counter: " << data[0] << ", last was: " << m_lastPacketCounter;
        finished(false);
        return;
    }
}

bool SportsSummaryOperation::finished(bool success)
{
    bool saved = true;
    if (success) {
        //store the successful samples
        //saved = saveSamples();
        m_settings.setValue("/uk/co/piggz/amazfish/device/lastactivitysyncmillis", m_sampleTime.toMSecsSinceEpoch());
        qDebug() << "finished fetch operation, last record was " << m_sampleTime;
    }
    return saved;
}

ActivitySummary *SportsSummaryOperation::parseSummary()
{
    ActivitySummary *summary = new ActivitySummary();

    QDataStream stream(m_buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    short version;
    stream >> version;
    summary->setVersion(version);

#if 0
    int activityKind = ActivityKind.TYPE_UNKNOWN;
    try {
        int rawKind = BLETypeConversions.toUnsigned(buffer.getShort());
        BipActivityType activityType = BipActivityType.fromCode(rawKind);
        activityKind = activityType.toActivityKind();
    } catch (Exception ex) {
        LOG.error("Error mapping acivity kind: " + ex.getMessage(), ex);
    }
    summary.setActivityKind(activityKind);

    // FIXME: should honor timezone we were in at that time etc
    long timestamp_start = BLETypeConversions.toUnsigned(buffer.getInt()) * 1000;
    long timestamp_end = BLETypeConversions.toUnsigned(buffer.getInt()) * 1000;


    // FIXME: should be done like this but seems to return crap when in DST
    //summary.setStartTime(new Date(timestamp_start));
    //summary.setEndTime(new Date(timestamp_end));

    // FIXME ... so do it like this
    long duration = timestamp_end - timestamp_start;
    summary.setStartTime(new Date(getLastStartTimestamp().getTimeInMillis()));
    summary.setEndTime(new Date(getLastStartTimestamp().getTimeInMillis() + duration));

    int baseLongitude = buffer.getInt();
    int baseLatitude = buffer.getInt();
    int baseAltitude = buffer.getInt();
    summary.setBaseLongitude(baseLongitude);
    summary.setBaseLatitude(baseLatitude);
    summary.setBaseAltitude(baseAltitude);
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

    buffer.getInt(); //
    buffer.getInt(); //
    buffer.getInt(); //

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

    buffer.getShort(); //
#endif
    return summary;
}

void SportsSummaryOperation::setStartDate(const QDateTime &sd)
{
    m_startDate = sd;
}

QDateTime SportsSummaryOperation::lastActivitySync()
{
    qlonglong ls = m_settings.value("/uk/co/piggz/amazfish/device/lastsummarysyncmillis").toLongLong();

    if (ls == 0) {
        return QDateTime::currentDateTime().addDays(-30);
    }
    QTimeZone tz = QTimeZone(QTimeZone::systemTimeZone().standardTimeOffset(QDateTime::currentDateTime())); //Getting the timezone without DST

    qDebug() << "last sync was " << ls << QDateTime::fromMSecsSinceEpoch(ls, tz);
    return QDateTime::fromMSecsSinceEpoch(ls, tz);
}
