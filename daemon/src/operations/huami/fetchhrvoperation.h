#ifndef FETCHHRVOPERATION_H
#define FETCHHRVOPERATION_H

#include "abstractfetchoperation.h"
#include "database.h"

class FetchHrvOperation:  public AbstractFetchOperation
{
public:
    FetchHrvOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:

    bool processBufferedData() override;
    bool saveRecords(QVector<Database::HrvRecord> recs);
};
#endif // FETCHHRVOPERATION_H
