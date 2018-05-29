#include "activityfetchoperation.h"

#include <QDebug>
#include <KDb3/KDbTransactionGuard>

ActivityFetchOperation::ActivityFetchOperation(const QDateTime &sd, KDbConnection *conn)
{
    m_startDate = sd;
    m_conn = conn;
}

void ActivityFetchOperation::newData(const QByteArray &data)
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
        m_settings.setValue("/uk/co/piggz/amazfish/device/lastactivitysyncmillis", m_sampleTime.toMSecsSinceEpoch());
        qDebug() << "finished fetch operation, last record was " << m_sampleTime;
    }
    return saved;
}

bool ActivityFetchOperation::saveSamples()
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

void ActivityFetchOperation::setStartDate(const QDateTime &sd)
{
    m_startDate = sd;
}
