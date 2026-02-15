#include "activitysampleex.h"

ActivitySampleEx::ActivitySampleEx(QDateTime d, int k, int i, int s, int h)
{
    m_datetime = d;
    m_kind = k;
    m_intensity = i;
    m_steps = s;
    m_heartRate = h;
}

QDateTime ActivitySampleEx::datetime() const
{
    return m_datetime;
}

int ActivitySampleEx::heartrate() const
{
    return m_heartRate;
}


int ActivitySampleEx::intensity() const
{
    return m_intensity;
}


int ActivitySampleEx::steps() const
{
    return m_steps;
}


int ActivitySampleEx::kind() const
{
    return m_kind;
}
