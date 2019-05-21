#include "activityfetchoperation.h"

#include <QDebug>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"

ActivityFetchOperation::ActivityFetchOperation(QBLEService *service, KDbConnection *conn) : AbstractFetchOperation(service)
{
    m_conn = conn;
    setLastSyncKey("/uk/co/piggz/amazfish/device/lastactivitysyncmillis");
}

void ActivityFetchOperation::start()
{
    setStartDate(lastActivitySync());


    qDebug() << "last activity sync was" << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_ACTIVTY) + rawDate);
    //Send log read command
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
}

void ActivityFetchOperation::handleData(const QByteArray &data)
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

bool ActivityFetchOperation::finished(bool success)
{
    bool saved = true;
    if (success) {
        //store the successful samples
        saved = saveSamples();
        saveLastActivitySync(m_sampleTime.toMSecsSinceEpoch());
        qDebug() << "finished fetch operation, last record was " << m_sampleTime;
    }
    return saved;
}

bool ActivityFetchOperation::saveSamples()
{
    bool saved = true;
    if (m_samples.count() > 0) {
        if (m_conn && m_conn->isDatabaseUsed()) {
            m_sampleTime = startDate();

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

