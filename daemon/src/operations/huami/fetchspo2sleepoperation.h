#ifndef FETCHSPO2SLEEPOPERATION_H
#define FETCHSPO2SLEEPOPERATION_H

#include "database.h"
#include <abstractfetchoperation.h>

class FetchSpo2SleepOperation : public AbstractFetchOperation
{
public:
    FetchSpo2SleepOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:

    bool processBufferedData() override;
    bool saveRecords(QVector<Database::Spo2SleepRecord> recs);
};

#endif // FETCHSPO2SLEEPOPERATION_H
