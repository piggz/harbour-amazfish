#ifndef ABSTRACTACTIVITYDETAILPARSER_H
#define ABSTRACTACTIVITYDETAILPARSER_H

#include <QByteArray>
#include "activitycoordinate.h"
#include "activitysummary.h"

class AbstractActivityDetailParser
{
public:
    virtual ~AbstractActivityDetailParser();

    static constexpr double HUAMI_TO_DECIMAL_DEGREES_DIVISOR = 3000000.0;

    virtual void parse(const QByteArray &bytes) = 0;
    void setSummary(const ActivitySummary &summary);
    QString toText();
    QString toTCX();
    double convertHuamiValueToDecimalDegrees(long huamiValue);


protected:
    QList<ActivityCoordinate> m_activityTrack;
    ActivitySummary m_summary;
};

#endif // ABSTRACTACTIVITYDETAILPARSER_H
