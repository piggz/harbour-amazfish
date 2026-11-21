#ifndef FETCHSPO2NORMALOPERATION_H
#define FETCHSPO2NORMALOPERATION_H

#include "abstractfetchoperation.h"
#include "database.h"

class FetchSpo2NormalOperation : public AbstractFetchOperation
{
public:
    FetchSpo2NormalOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:

    bool processBufferedData() override;
    bool saveRecords(QVector<Database::Spo2Record> recs);
};

#endif // FETCHSPO2NORMALOPERATION_H
