#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QVariant>

#include <KDb3/KDbConnection>

class DataSource: public QObject
{
    Q_OBJECT

public:
    enum Type {
        Heartrate = 1,
        Steps = 2,
        Sleep = 3,
        Intensity = 4,
        StepSummary = 5,
        SleepSummary = 6,
        BatteryLog = 7,
        HRV = 8,
        Spo2Normal = 9,
        Spo2Sleep = 10,
        BodyTemperature = 11,
        StressAuto = 12,
        StressManual = 13,
        StressSummary = 14,
        Activity = 15, // per-sample day data: x, i (intensity %), k (sleep phase), h (heartrate), s (steps)
        SleepPhases = 16 // per-sample data of the night ending on 'day' (12:00 -> 12:00)
    };

    // Sleep phase classification returned in the 'k' field of Activity / SleepPhases samples
    enum Phase {
        PhaseAwake = 0,
        PhaseLightSleep = 1,
        PhaseDeepSleep = 2
    };
    Q_ENUM(Phase)
    Q_ENUM(Type)

    DataSource();
    void setConnection(KDbConnection *conn);

    Q_INVOKABLE QVariant data(const DataSource::Type type, const QDate  &day);

private:
    KDbConnection *m_conn = nullptr;
    
    struct SleepSession {
        QDateTime sleepStart;
        QDateTime sleepEnd;
        long lightSleepDuration;
        long deepSleepDuration;
    };
    QList<SleepSession> calculateSleep(const QDate &day);
    QList<QVariant> activitySamples(const QDateTime& start, const QDateTime& end);
    bool isSleep(int kind);
};

#endif // DATASOURCE_H
