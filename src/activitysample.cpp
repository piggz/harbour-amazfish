#include "activitysample.h"

ActivitySample::ActivitySample(int k, int i, int s, int h)
{
    m_kind = k;
    m_intensity = i;
    m_steps = s;
    m_heartRate = h;
}

void ActivitySample::write(QTextStream &ts)
{
    ts << m_kind << "," << m_intensity << "," << m_steps << "," << m_heartRate;
}
