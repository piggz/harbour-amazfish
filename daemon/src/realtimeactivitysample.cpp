#include "realtimeactivitysample.h"

#include <QDebug>

RealtimeActivitySample::RealtimeActivitySample(QObject* parent) : QObject(parent) {
    m_lastSync = QDateTime::currentDateTime();
}

void RealtimeActivitySample::setKind(int _kind) {
    if (_kind == 0) {
        return;
    }
    sampleUpdated();
    m_kind = _kind;
}

void RealtimeActivitySample::setIntensity(int _intensity) {
    if (_intensity == 0) {
        return;
    }
    sampleUpdated();
    m_intensity = _intensity;
}

void RealtimeActivitySample::setSteps(int _steps) {
    if (_steps == 0) {
        return;
    }
    sampleUpdated();
    m_steps = _steps;
}

void RealtimeActivitySample::setHeartrate(int _heartrate) {
    if (_heartrate == 0) {
        return;
    }
    sampleUpdated();
    m_heartrate_samples.push_back(_heartrate);
}

void RealtimeActivitySample::sampleUpdated() {
    QDateTime now = QDateTime::currentDateTime();


    int steps_diff = 0;
    int avg_heartrate = 0;

    if (m_steps_previous == 0) {
        m_steps_previous = m_steps;
    }



    // send old data
    if (m_lastSync.secsTo(now) > m_interval) {

        if ((m_steps_previous != 0) && (m_steps > m_steps_previous)) {
            steps_diff = m_steps - m_steps_previous;
        }

        if (!m_heartrate_samples.isEmpty()) {
            for (int value : m_heartrate_samples) {
                avg_heartrate += value;
            }
            avg_heartrate = avg_heartrate / m_heartrate_samples.size();
        }
        if ((m_intensity != 0) || (steps_diff != 0) || (avg_heartrate != 0)) {
            emit samplesReady(m_lastSync, m_kind, m_intensity, steps_diff, avg_heartrate);

            // prepare for new data
            m_kind = 0;
            m_intensity = 0;
            m_steps_previous = m_steps;
            m_steps = 0;
            m_heartrate_samples.clear();
            m_lastSync = now;

        }
    }


}

void RealtimeActivitySample::slot_informationChanged(AbstractDevice::Info infokey, const QString &infovalue) {

    switch (infokey) {
        case AbstractDevice::INFO_STEPS:
            setSteps(infovalue.toInt());
        break;
        case AbstractDevice::INFO_HEARTRATE:
            setHeartrate(infovalue.toInt());
        break;
        default:
            qWarning() << Q_FUNC_INFO << "unexpeted key " << infokey;
        break;
    }

}