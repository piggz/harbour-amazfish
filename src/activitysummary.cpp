#include "activitysummary.h"

ActivitySummary::ActivitySummary()
{

}

void ActivitySummary::setVersion(short v)
{
    m_version=v;
}

void ActivitySummary::setActivityKind(ActivityKind::Type type)
{
    m_activityKind = type;
}


ActivityKind::Type ActivitySummary::activitKind()
{
    return m_activityKind;
}   