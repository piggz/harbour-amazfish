#ifndef REALTIME_ACTIVITY_SAMPLE__H
#define REALTIME_ACTIVITY_SAMPLE__H

#include <QObject>
#include <QDateTime>
#include "abstractdevice.h"

class RealtimeActivitySample : public QObject {
    Q_OBJECT

    public:
        RealtimeActivitySample(QObject* parent = NULL);

        void setKind(int _kind);
        void setIntensity(int _intensity);
        void setSteps(int _steps);
        void setHeartrate(int _heartrate);

    signals:
        void samplesReady(QDateTime dt, int kind, int intensity, int steps, int heart);

    public slots:
    Q_SLOT void slot_informationChanged(AbstractDevice::Info infokey, const QString &infovalue);


    private:
        int m_kind = 0;
        int m_intensity = 0;
        int m_steps = 0;
        int m_steps_previous = 0;
        QList<int> m_heartrate_samples;

        QDateTime m_lastSync;
        qint64 m_interval = 60;

        void sampleUpdated();
};

#endif //  REALTIME_ACTIVITY_SAMPLE__H
