#include "activityfetchoperation.h"

#include <QDebug>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"

ActivityFetchOperation::ActivityFetchOperation(QBLEService *service, KDbConnection *conn, int sampleSize) : AbstractFetchOperation(service), m_conn(conn), m_sampleSize(sampleSize)
{
    setLastSyncKey("device/lastactivitysyncmillis");
}

void ActivityFetchOperation::start()
{
    setStartDate(lastActivitySync());

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, false);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_ACTIVTY) + rawDate);
}

void ActivityFetchOperation::handleData(const QByteArray &data)
{
    int len = data.length();

    if (len % m_sampleSize != 1) {
        qDebug() << Q_FUNC_INFO << "Unexpected data size";
        return;
    }

    for (int i = 1; i < len; i+=m_sampleSize) {
        ActivitySample sample(data[i] & 0xff, data[i + 1] & 0xff, data[i + 2] & 0xff, data[i + 3] & 0xff);
        if (m_sampleSize == 8) {
            qDebug() << Q_FUNC_INFO << "Sample data missed:" << (data[i + 4] & 0xff) << (data[i + 5] & 0xff) << (data[i + 6] & 0xff) << (data[i + 7] & 0xff);
        }
        m_samples << (sample);
    }
}

bool ActivityFetchOperation::finished(bool success)
{
    bool saved = true;
    if (success) {
        //store the successful samples
        saved = saveSamples();
        m_sampleTime.setTimeSpec(Qt::LocalTime);
        qDebug() << Q_FUNC_INFO << "Last sample time saved as " << m_sampleTime.toString() << m_sampleTime.offsetFromUtc() <<  m_sampleTime.toMSecsSinceEpoch();

        saveLastActivitySync(m_sampleTime.toMSecsSinceEpoch());
        qDebug() << Q_FUNC_INFO << "Last record was " << m_sampleTime;
    }
    return saved;
}

bool ActivityFetchOperation::saveSamples()
{
    if (m_samples.count() <= 0) {
        return true;
    }

    if (!m_conn || !(m_conn->isDatabaseUsed())) {
        qDebug() << Q_FUNC_INFO << "Database not connected";
        return false;
    }

    m_sampleTime = startDate();
    qDebug() << Q_FUNC_INFO << "Start sample time" << m_sampleTime;

    auto config = AmazfishConfig::instance();
    uint id = qHash(config->profileName());
    uint devid = qHash(config->pairedAddress());

    KDbTransaction transaction = m_conn->beginTransaction();
    KDbTransactionGuard tg(transaction);

    KDbFieldList fields;
    auto mibandActivity = m_conn->tableSchema("mi_band_activity");

    fields.addField(mibandActivity->field("timestamp"));
    fields.addField(mibandActivity->field("timestamp_dt"));
    fields.addField(mibandActivity->field("device_id"));
    fields.addField(mibandActivity->field("user_id"));
    fields.addField(mibandActivity->field("raw_intensity"));
    fields.addField(mibandActivity->field("steps"));
    fields.addField(mibandActivity->field("raw_kind"));
    fields.addField(mibandActivity->field("heartrate"));

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
            qDebug() << Q_FUNC_INFO << "error inserting record";
            return false;
        }
        m_sampleTime = m_sampleTime.addSecs(60);
    }
    tg.commit();

    return true;
}

