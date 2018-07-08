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
    setLastSyncKey("/uk/co/piggz/amazfish/device/lastsportsyncmillis");
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
    //Send log read command
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
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
        m_summary = parseSummary();
        saved = saveSummary();
        m_success = saved;
    }
    return saved;
}

ActivitySummary SportsSummaryOperation::parseSummary()
{
    ActivitySummary summary;
    qDebug() << "Buffer:" << m_buffer.toHex();

    QDataStream stream(m_buffer);
    stream.setByteOrder(QDataStream::LittleEndian);

    quint16 version;
    stream >> version;
    summary.setVersion(version);

    quint16 kind;
    stream >> kind;

    qDebug() << "raw kind" << kind;
    ActivityKind::Type  activityKind = ActivityKind::Unknown;
    activityKind = ActivityKind::fromBipType(kind);
    summary.setActivityKind(activityKind);

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
    summary.setStartTime(startDate());
    summary.setEndTime(startDate().addSecs(duration));


    qint32 baseLongitude = 0;
    qint32 baseLatitude = 0;
    qint32 baseAltitude = 0;
    stream >> baseLongitude >> baseLatitude >> baseAltitude;

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

bool SportsSummaryOperation::saveSummary()
{
    bool saved = true;
    if (m_conn && m_conn->isDatabaseUsed()) {
        m_sampleTime = startDate();

        uint id = qHash(m_settings.value("/uk/co/piggz/amazfish/profile/name").toString());
        uint devid = qHash(m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString());

        KDbTransaction transaction = m_conn->beginTransaction();
        KDbTransactionGuard tg(transaction);

        KDbFieldList fields;

        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("version"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("start_timestamp"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("start_timestamp_dt"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("end_timestamp"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("end_timestamp_dt"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("device_id"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("user_id"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("kind"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_longitude"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_latitude"));
        fields.addField(m_conn->tableSchema("mi_band_sports_summary")->field("base_altitude"));


        QList<QVariant> values;
        values << m_summary.version();
        values << m_summary.startTime().toMSecsSinceEpoch() / 1000;
        values << m_summary.startTime();
        values << m_summary.endTime().toMSecsSinceEpoch() / 1000;
        values << m_summary.endTime();
        values << devid;
        values << id;
        values << m_summary.activityKind();
        values << m_summary.baseLongitude() / 3000000.0;
        values << m_summary.baseLatitude() / 3000000.0;
        values << m_summary.baseAltitude();

        qDebug() << "Saving sports data" << values;

        QSharedPointer<KDbSqlResult> result = m_conn->insertRecord(&fields, values);

        if (!result->lastResult().isError()) {
            long lastId = result->lastInsertRecordId();
            m_summary.setId(lastId);
            qDebug() << "Record Id is" << m_summary.id();
        } else {
            qDebug() << "error inserting record";
            saved = false;
        }

        tg.commit();
    } else {
        qDebug() << "Database not connected";
        saved = false;
    }

    return saved;
}

bool SportsSummaryOperation::success() const
{
    return m_success;
}

ActivitySummary SportsSummaryOperation::summary()
{
    return m_summary;
}
