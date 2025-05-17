#ifndef ABSTRACTACTIVITYSUMMARYPARSER_H
#define ABSTRACTACTIVITYSUMMARYPARSER_H

#include <QByteArray>
#include "activitysummary.h"

class AbstractActivitySummaryParser {

public:
    virtual ~AbstractActivitySummaryParser() {};
    virtual ActivitySummary parseBinaryData(const QByteArray &data, const QDateTime &startTime) = 0;
};

#endif // ABSTRACTACTIVITYSUMMARYPARSER_H
