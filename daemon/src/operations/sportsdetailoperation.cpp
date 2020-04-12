#include "sportsdetailoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"
#include "bipactivitydetailparser.h"
#include "amazfishconfig.h"

SportsDetailOperation::SportsDetailOperation(QBLEService *service, KDbConnection *conn, const ActivitySummary &summary) : AbstractFetchOperation(service)
{
    m_conn = conn;
    m_summary = summary;
    setLastSyncKey("device/lastsportsyncmillis");
}

void SportsDetailOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0, false);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_DETAILS) + rawDate);
}

void SportsDetailOperation::handleData(const QByteArray &data)
{
    if (data.length() < 2) {
        qDebug() << "unexpected sports summary data length: " << data.length();
        return;
    }

    if ((m_lastPacketCounter + 1) == (int)data[0] ) {
        m_lastPacketCounter++;
        if (m_lastPacketCounter >= 255) {
            m_lastPacketCounter =-1;
        }
        m_buffer += data.mid(1);
    } else {
        qDebug() << "invalid package counter: " << (int)data[0] << ", last was: " << m_lastPacketCounter;
        finished(false);
        return;
    }
}

bool SportsDetailOperation::finished(bool success)
{
    bool saved = false;
    if (success) {
        BipActivityDetailParser parser(m_summary);
        parser.parse(m_buffer);
        m_gpx = parser.toText();

        QDir cachelocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString filename = m_summary.name() + ".gpx";
        QFile logFile;

        QDir laufhelden(QDir::homePath() + "/Laufhelden/");

        qDebug() << "Checking for " << laufhelden.absolutePath();

        if (laufhelden.exists()) {
            logFile.setFileName(laufhelden.absolutePath() + "/" + filename);
        } else {
            logFile.setFileName(cachelocation.absolutePath() + "/" + filename);
        }

        if(logFile.open(QIODevice::WriteOnly)) {
            qDebug() << "Saving to" << logFile.fileName();
            QTextStream stream( &logFile );
            stream << m_gpx;
        }

        saved = saveSport();
        //Convert local to UTC (without offsetting) to save as the last sync time
        //QDateTime temp = QDateTime::fromString(m_summary.endTime().toLocalTime().toString("yyyy-MM-dd hh:mm:ss"), "yyyy-MM-dd hh:mm:ss");
        //temp.setTimeSpec(Qt::UTC);
        qDebug() << "End sport time is:" << m_summary.endTime() << m_summary.endTime().toMSecsSinceEpoch();
        if (m_summary.endTime().isDaylightTime()) {
            qDebug() << "Adding DST offset to last save time";
            m_summary.setEndTime(m_summary.endTime().addSecs(3600));
            qDebug() << "End sport time is:" << m_summary.endTime() << m_summary.endTime().toMSecsSinceEpoch();
        }
        saveLastActivitySync(m_summary.endTime().toMSecsSinceEpoch());
    } else {
        setAbort(true);
    }
    return saved;
}

bool SportsDetailOperation::saveSport()
{
    if (!m_conn || !m_conn->isDatabaseUsed()) {
        qDebug() << "Database not connected";
        return false;
    }

    auto config = AmazfishConfig::instance();
    uint id = qHash(config->profileName());
    uint devid = qHash(config->pairedAddress());

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto sportsData = m_conn->tableSchema("sports_data");

    fields.addField(sportsData->field("name"));
    fields.addField(sportsData->field("version"));
    fields.addField(sportsData->field("start_timestamp"));
    fields.addField(sportsData->field("start_timestamp_dt"));
    fields.addField(sportsData->field("end_timestamp"));
    fields.addField(sportsData->field("end_timestamp_dt"));
    fields.addField(sportsData->field("device_id"));
    fields.addField(sportsData->field("user_id"));
    fields.addField(sportsData->field("kind"));
    fields.addField(sportsData->field("base_longitude"));
    fields.addField(sportsData->field("base_latitude"));
    fields.addField(sportsData->field("base_altitude"));
    fields.addField(sportsData->field("gpx"));


    QList<QVariant> values;
    values << m_summary.name();
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
    values << m_gpx;

    qDebug() << "Saving Sport:" << m_summary.name()  << m_summary.startTime() << m_summary.startTime().toString(Qt::ISODate);

    QSharedPointer<KDbSqlResult> result = m_conn->insertRecord(&fields, values);

    if (result->lastResult().isError()) {
        qDebug() << "error inserting record";
        return false;
    }

    long lastId = result->lastInsertRecordId();
    m_summary.setId(lastId);
    qDebug() << "Record Id is" << m_summary.id();

    tg.commit();
    return true;
}
