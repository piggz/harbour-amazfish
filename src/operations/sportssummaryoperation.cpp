#include "sportssummaryoperation.h"

#include <QDebug>
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
    int len = data.length();

    if (len % 4 != 1) {
        qDebug() << "Unexpected data size";
        return;
    }

    for (int i = 1; i < len; i+=4) {
        ActivitySample sample(data[i] & 0xff, data[i + 1] & 0xff, data[i + 2] & 0xff, data[i + 3] & 0xff);
        m_samples << (sample);
    }
}

bool SportsSummaryOperation::finished(bool success)
{
    bool saved = true;
    if (success) {
        //store the successful samples
        saved = saveSamples();
        m_settings.setValue("/uk/co/piggz/amazfish/device/lastactivitysyncmillis", m_sampleTime.toMSecsSinceEpoch());
        qDebug() << "finished fetch operation, last record was " << m_sampleTime;
    }
    return saved;
}

bool SportsSummaryOperation::saveSamples()
{
    bool saved = true;
    if (m_samples.count() > 0) {
        if (m_conn && m_conn->isDatabaseUsed()) {
            m_sampleTime = m_startDate;

            uint id = qHash(m_settings.value("/uk/co/piggz/amazfish/profile/name").toString());
            uint devid = qHash(m_settings.value("/uk/co/piggz/amazfish/pairedAddress").toString());

            KDbTransaction transaction = m_conn->beginTransaction();
            KDbTransactionGuard tg(transaction);

            KDbFieldList fields;

            fields.addField(m_conn->tableSchema("mi_band_activity")->field("timestamp"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("timestamp_dt"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("device_id"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("user_id"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("raw_intensity"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("steps"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("raw_kind"));
            fields.addField(m_conn->tableSchema("mi_band_activity")->field("heartrate"));

            for (int i = 0; i < m_samples.count(); ++i) {
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
                m_sampleTime = m_sampleTime.addSecs(60);
            }
            tg.commit();
        } else {
            qDebug() << "Database not connected";
            saved = false;
        }
    }
    return saved;
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
