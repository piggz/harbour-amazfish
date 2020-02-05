#include "sportsdetailoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"
#include "bipactivitydetailparser.h"

SportsDetailOperation::SportsDetailOperation(QBLEService *service, KDbConnection *conn, const ActivitySummary &summary) : AbstractFetchOperation(service)
{
    m_conn = conn;
    m_summary = summary;
    setLastSyncKey("/uk/co/piggz/amazfish/device/lastsportsyncmillis");
}

void SportsDetailOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0);

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
        QDateTime temp = QDateTime::fromString(m_summary.endTime().toLocalTime().toString("yyyy-MM-dd hh:mm:ss"), "yyyy-MM-dd hh:mm:ss");
        temp.setTimeSpec(Qt::UTC);
        saveLastActivitySync(temp.toMSecsSinceEpoch());
    } else {
        setAbort(true);
    }
    return saved;
}

bool SportsDetailOperation::saveSport()
{
    bool saved = true;
    if (m_conn && m_conn->isDatabaseUsed()) {
        uint id = qHash(m_settings.value("/uk/co/piggz/amazfish/profile/name").toString());
        uint devid = qHash(m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString());

        KDbTransaction transaction = m_conn->beginTransaction();
        KDbTransactionGuard tg(transaction);

        KDbFieldList fields;

        fields.addField(m_conn->tableSchema("sports_data")->field("name"));
        fields.addField(m_conn->tableSchema("sports_data")->field("version"));
        fields.addField(m_conn->tableSchema("sports_data")->field("start_timestamp"));
        fields.addField(m_conn->tableSchema("sports_data")->field("start_timestamp_dt"));
        fields.addField(m_conn->tableSchema("sports_data")->field("end_timestamp"));
        fields.addField(m_conn->tableSchema("sports_data")->field("end_timestamp_dt"));
        fields.addField(m_conn->tableSchema("sports_data")->field("device_id"));
        fields.addField(m_conn->tableSchema("sports_data")->field("user_id"));
        fields.addField(m_conn->tableSchema("sports_data")->field("kind"));
        fields.addField(m_conn->tableSchema("sports_data")->field("base_longitude"));
        fields.addField(m_conn->tableSchema("sports_data")->field("base_latitude"));
        fields.addField(m_conn->tableSchema("sports_data")->field("base_altitude"));
        fields.addField(m_conn->tableSchema("sports_data")->field("gpx"));


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
