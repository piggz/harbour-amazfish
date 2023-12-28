#ifndef DATASOURCE_H
#define DATASOURCE_H

#include <QObject>
#include <QVariant>

#include <KDb3/KDbConnection>
#include "amazfish.h"

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
    };
    Q_ENUM(Type)

    DataSource();
    void setConnection(KDbConnection *conn);

    Q_INVOKABLE QVariant data(DataSource::Type type, const QDate  &day);

private:
    KDbConnection *m_conn = nullptr;
    
    struct SleepSession {
        QDateTime sleepStart;
        QDateTime sleepEnd;
        long lightSleepDuration;
        long deepSleepDuration;
    };
    QList<SleepSession> calculateSleep(const QDate &day);
    bool isSleep(int kind);
};

#endif // DATASOURCE_H
