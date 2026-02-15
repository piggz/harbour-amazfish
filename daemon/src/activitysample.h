#ifndef ACTIVITYSAMPLE_H
#define ACTIVITYSAMPLE_H

#include <QTextStream>
#include <QDateTime>

class ActivitySample
{
public:
    ActivitySample(int kind, int intensity, int steps, int heartrate);
    ActivitySample(QDateTime datetime, int kind, int intensity, int steps, int heartrate);

    QDateTime datetime() const;
    int kind() const;
    int intensity() const;
    int steps() const;
    int heartrate() const;

private:
    QDateTime m_datetime;
    int m_kind = 0;
    int m_intensity = 0;
    int m_steps = 0;
    int m_heartRate = 0;
};

#endif // ACTIVITYSAMPLE_H
