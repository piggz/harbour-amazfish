#ifndef FETCHSTRESSAUTOOPERATION_H
#define FETCHSTRESSAUTOOPERATION_H

#include "database.h"
#include <abstractfetchoperation.h>

class FetchStressAutoOperation : public AbstractFetchOperation
{
public:
    FetchStressAutoOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    bool processBufferedData() override;
    bool saveRecords(QVector<Database::StressRecord> recs, QDateTime lastTimeDefault);
};

#endif // FETCHSTRESSAUTOOPERATION_H
