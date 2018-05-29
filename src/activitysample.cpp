#include "activitysample.h"

ActivitySample::ActivitySample(int k, int i, int s, int h)
{
    m_kind = k;
    m_intensity = i;
    m_steps = s;
    m_heartRate = h;
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
