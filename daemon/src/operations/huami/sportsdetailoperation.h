#ifndef SPORTSDETAILOPERATION_H
#define SPORTSDETAILOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "abstractactivitydetailparser.h"
#include "abstractfetchoperation.h"
#include "activitysummary.h"

class SportsDetailOperation : public AbstractFetchOperation
{
public:
    SportsDetailOperation(HuamiFetcher* fetcher, const ActivitySummary& summary, AbstractActivityDetailParser* parser = nullptr, bool isZeppOs = false);
    ~SportsDetailOperation();

    void start() override;

private:
    ActivitySummary m_summary;
    AbstractActivityDetailParser *m_parser = 0;
    QString m_gpx;

    bool saveSport(const QString &filename);
    bool save();
    bool processBufferedData() override;
};

#endif // SPORTSDETAILOPERATION_H
