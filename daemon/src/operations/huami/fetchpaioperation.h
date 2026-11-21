#ifndef FETCHPAIOPERATION_H
#define FETCHPAIOPERATION_H

#include <QDateTime>

#include "abstractfetchoperation.h"
#include "database.h"

class FetchPaiOperation : public AbstractFetchOperation
{
public:
    FetchPaiOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:

    bool processBufferedData() override;
    bool saveRecords(QVector<Database::PaiRecord> recs);
};

#endif // FETCHPAIOPERATION_H
