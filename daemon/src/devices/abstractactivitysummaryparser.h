#ifndef ABSTRACTACTIVITYSUMMARYPARSER_H
#define ABSTRACTACTIVITYSUMMARYPARSER_H

#include <QByteArray>
#include "activitysummary.h"

class AbstractActivitySummaryParser {

public:
    static constexpr double HUAMI_TO_DECIMAL_DEGREES_DIVISOR = 3000000.0;

    virtual ~AbstractActivitySummaryParser() {};
    virtual ActivitySummary parseBinaryData(const QByteArray &data, const QDateTime &startTime) = 0;
    double convertHuamiValueToDecimalDegrees(long huamiValue)
    {
        return huamiValue / HUAMI_TO_DECIMAL_DEGREES_DIVISOR;
    }

};

#endif // ABSTRACTACTIVITYSUMMARYPARSER_H
