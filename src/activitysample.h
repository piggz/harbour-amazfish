#ifndef ACTIVITYSAMPLE_H
#define ACTIVITYSAMPLE_H

#include <QTextStream>

class ActivitySample
{
public:
    ActivitySample(int k, int i, int s, int h);
    void write(QTextStream &ts);

private:
    int m_kind = 0;
    int m_intensity = 0;
    int m_steps = 0;
    int m_heartRate = 0;
};

#endif // ACTIVITYSAMPLE_H
