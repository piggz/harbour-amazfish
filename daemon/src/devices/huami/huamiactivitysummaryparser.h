#ifndef HUAMIACTIVITYSUMMARYPARSER_H
#define HUAMIACTIVITYSUMMARYPARSER_H

#include <abstractactivitysummaryparser.h>

class HuamiActivitySummaryParser : public AbstractActivitySummaryParser
{
public:
    HuamiActivitySummaryParser();
    ActivitySummary parseBinaryData(const QByteArray& data, const QDateTime &startTime) override;
};

#endif // HUAMIACTIVITYSUMMARYPARSER_H
