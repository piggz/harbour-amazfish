#include "activityfetchoperation.h"

#include <QDebug>

#include "mibandservice.h"
#include "typeconversion.h"
#include "amazfishconfig.h"
#include "huami/huamifetcher.h"

#include "database.h"

ActivityFetchOperation::ActivityFetchOperation(HuamiFetcher* fetcher, int sampleSize, bool isZeppOs)
    : AbstractFetchOperation(fetcher, isZeppOs)
    , m_sampleSize(sampleSize)
{
    setLastSyncKey("device/lastactivitysyncmillis");
}

void ActivityFetchOperation::start()
{
    qDebug() << Q_FUNC_INFO;
    setStartDate(lastActivitySync());

    m_fetcher->setNotifications(true, true);

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDateLocal();

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDateLocal(), 0, true);

    //Send log read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_ACTIVTY) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool ActivityFetchOperation::saveSamples()
{
    qDebug()<< Q_FUNC_INFO << m_sampleSize << m_buffer.length();

    int len = m_buffer.length();

    if (len % m_sampleSize != 0) {
        qDebug() << Q_FUNC_INFO << "Unexpected data size";
        m_valid = false;
        return false;
    }

    for (int i = 0; i < len; i+=m_sampleSize) {
        ActivitySample sample(m_buffer[i] & 0xff, m_buffer[i + 1] & 0xff, m_buffer[i + 2] & 0xff, m_buffer[i + 3] & 0xff);
        if (m_sampleSize == 8) {
            qDebug() << Q_FUNC_INFO << "Sample data missed:" << (m_buffer[i + 4] & 0xff) << (m_buffer[i + 5] & 0xff) << (m_buffer[i + 6] & 0xff) << (m_buffer[i + 7] & 0xff);
        }
        m_samples << (sample);
    }

    if (m_samples.count() <= 0) {
        return true;
    }

    m_sampleTime = startDateLocal();
    qDebug() << Q_FUNC_INFO << "Start sample time" << m_sampleTime;

    // The watch sends one sample per minute without a time.
    QList<ActivitySample> samples;
    for (int i = 0; i < m_samples.count(); ++i) {
        samples << ActivitySample(m_sampleTime, m_samples[i].kind(), m_samples[i].intensity(), m_samples[i].steps(),
            m_samples[i].heartrate());
        m_sampleTime = m_sampleTime.addSecs(60);
    }

    return Database::storeActivitySamples(samples);
}

bool ActivityFetchOperation::processBufferedData()
{
    qDebug()<< Q_FUNC_INFO;

    //store the successful samples
    if (saveSamples()) {
        qDebug() << Q_FUNC_INFO << "Last sample time saved as " << m_sampleTime.toString() << m_sampleTime.offsetFromUtc() <<  m_sampleTime.toMSecsSinceEpoch();

        saveLastActivitySync(m_sampleTime.toMSecsSinceEpoch());
        qDebug() << Q_FUNC_INFO << "Last record was " << m_sampleTime;

        return true;
    }

    return false;
}

