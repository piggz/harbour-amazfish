#ifndef SPORTSSUMMARYOPERATION_H
#define SPORTSSUMMARYOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractactivitysummaryparser.h"
#include "abstractfetchoperation.h"
#include "activitysummary.h"

class SportsSummaryOperation : public AbstractFetchOperation
{
public:
    SportsSummaryOperation(HuamiFetcher *fetcher, KDbConnection *db, AbstractActivitySummaryParser *parser = nullptr, bool isZeppOs = false);
    ~SportsSummaryOperation();

    void start(QBLEService *service) override;

    bool success() const;
    ActivitySummary summary();

private:
    QDateTime m_sampleTime;
    bool m_error = false;
    ActivitySummary m_summary;

    bool processBufferedData() override;

    KDbConnection *m_conn;
    AbstractActivitySummaryParser *m_parser = nullptr;
};

#endif // SPORTSSUMMARYOPERATION_H
