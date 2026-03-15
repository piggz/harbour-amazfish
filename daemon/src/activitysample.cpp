#include "activitysample.h"

ActivitySample::ActivitySample(QDateTime d, int k, int i, int s, int h)
{
    m_datetime = d;
    m_kind = k;
    m_intensity = i;
    m_steps = s;
    m_heartRate = h;
}

ActivitySample::ActivitySample(int k, int i, int s, int h)
{
    m_kind = k;
    m_intensity = i;
    m_steps = s;
    m_heartRate = h;
}

QDateTime ActivitySample::datetime() const
{
    return m_datetime;
}

int ActivitySample::heartrate() const
{
    return m_heartRate;
}


int ActivitySample::intensity() const
{
    return m_intensity;
}


int ActivitySample::steps() const
{
    return m_steps;
}


int ActivitySample::kind() const
{
    return m_kind;
}

QDebug operator<<(QDebug debug, const ActivitySample &sample)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "ActivitySample("
                    << "datetime=" << sample.datetime() << ", "
                    << "kind=" << sample.kind() << ", "
                    << "intensity=" << sample.intensity() << ", "
                    << "steps=" << sample.steps() << ", "
                    << "heartrate=" << sample.heartrate()
                    << ")";
    return debug;
}
