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
        StressSummary = 14
    };
    Q_ENUM(Type)

    DataSource();
    void setConnection(KDbConnection *conn);

    Q_INVOKABLE QVariant data(const DataSource::Type type, const QDate  &day);

    struct SleepSession {
        QDateTime sleepStart;
        QDateTime sleepEnd;
        long lightSleepDuration;
        long deepSleepDuration;
    };

protected:
     virtual QList<SleepSession> calculateSleep(const QDate &day);
    KDbConnection *m_conn = nullptr;

private:
    



};

#endif // DATASOURCE_H
