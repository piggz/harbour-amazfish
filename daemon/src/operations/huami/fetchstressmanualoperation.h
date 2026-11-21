#ifndef FETCHSTRESSMANUALOPERATION_H
#define FETCHSTRESSMANUALOPERATION_H

#include "database.h"
#include <abstractfetchoperation.h>

class FetchStressManualOperation : public AbstractFetchOperation
{
public:
    FetchStressManualOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    bool processBufferedData() override;
    bool saveRecords(QVector<Database::StressRecord> recs, QDateTime lastTimeDefault);
};

#endif // FETCHSTRESSAUTOOPERATION_H
