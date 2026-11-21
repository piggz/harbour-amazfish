#ifndef FETCHHRVOPERATION_H
#define FETCHHRVOPERATION_H

#include "abstractfetchoperation.h"

class FetchHrvOperation:  public AbstractFetchOperation
{
public:
    FetchHrvOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct HrvRecord {
        QDateTime timestamp;
        int value;
    };

    bool processBufferedData() override;
    bool saveRecords(QVector<HrvRecord> recs);
};
#endif // FETCHHRVOPERATION_H
