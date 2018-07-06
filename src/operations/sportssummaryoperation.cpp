#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"

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
        ActivitySummary *summary = parseSummary();
        //save summary here
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

    short kind;
    stream >> kind;
    ActivityKind::Type  activityKind = ActivityKind::Unknown;
    activityKind = ActivityKind::fromBipType(kind);
    summary->setActivityKind(activityKind);

    // FIXME: should honor timezone we were in at that time etc
    int timestamp_start = 0;
    int timestamp_end = 0;
    
    stream >> timestamp_start;
    stream >> timestamp_end;

    // FIXME: should be done like this but seems to return crap when in DST
    //summary.setStartTime(new Date(timestamp_start));
    //summary.setEndTime(new Date(timestamp_end));

    // FIXME ... so do it like this
    long duration = timestamp_end - timestamp_start;
    //summary->setStartTime(new Date(getLastStartTimestamp().getTimeInMillis()));
    //summary->setEndTime(new Date(getLastStartTimestamp().getTimeInMillis() + duration));
    summary->setStartTime(m_startDate);
    summary->setEndTime(m_startDate.addSecs(duration));
        
  
    int baseLongitude = 0;
    int baseLatitude = 0;
    int baseAltitude = 0;
    stream >> baseLongitude >> baseLatitude >> baseAltitude;
   
    summary->setBaseLongitude(baseLongitude);
    summary->setBaseLatitude(baseLatitude);
    summary->setBaseAltitude(baseAltitude);
    

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

    qDebug() << "last sports  sync was " << ls << QDateTime::fromMSecsSinceEpoch(ls, tz);
    return QDateTime::fromMSecsSinceEpoch(ls, tz);
}

bool SportsSummaryOperation::saveSummary(const ActivitySummary &summary)
{
    bool saved = true;
        if (m_conn && m_conn->isDatabaseUsed()) {
            m_sampleTime = m_startDate;

            uint id = qHash(m_settings.value("/uk/co/piggz/amazfish/profile/name").toString());
            uint devid = qHash(m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString());

            KDbTransaction transaction = m_conn->beginTransaction();
            KDbTransactionGuard tg(transaction);

            KDbFieldList fields;

            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("id"))
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("version"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("start_timestamp"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("start_timestamp_dt"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("start_timestamp"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("end_timestamp_dt"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("device_id"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("user_id"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("kind"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_longitude"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_latitude"));
            fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_altitude"));

            
                QList<QVariant> values;
                values << m_sampleTime.toMSecsSinceEpoch() / 1000;
                values << m_sampleTime;
                values << devid;
                values << id;
                values << m_samples[i].intensity();
                values << m_samples[i].steps();
                values << m_samples[i].kind();
                values << m_samples[i].heartrate();

                if (!m_conn->insertRecord(&fields, values)) {
                    qDebug() << "error inserting record";
                    saved = false;
                    break;
                }
               
            tg.commit();
        } else {
            qDebug() << "Database not connected";
            saved = false;
        }
    
    return saved;
}

