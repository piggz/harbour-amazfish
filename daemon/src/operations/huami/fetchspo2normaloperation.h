#ifndef FETCHSPO2NORMALOPERATION_H
#define FETCHSPO2NORMALOPERATION_H

#include "abstractfetchoperation.h"

class FetchSpo2NormalOperation : public AbstractFetchOperation
{
public:
    FetchSpo2NormalOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct Spo2Record {
        QDateTime timestamp;
        bool automatic;
        uint8_t  value;
    };

    bool processBufferedData() override;
    bool saveRecords(QVector<Spo2Record> recs);
};

#endif // FETCHSPO2NORMALOPERATION_H
