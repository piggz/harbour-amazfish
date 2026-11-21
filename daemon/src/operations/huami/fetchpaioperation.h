#ifndef FETCHPAIOPERATION_H
#define FETCHPAIOPERATION_H

#include <QDateTime>

#include "abstractfetchoperation.h"

class FetchPaiOperation : public AbstractFetchOperation
{
public:
    FetchPaiOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct PaiRecord {
        QDate day;
        float low;
        float moderate;
        float high;
        uint16_t time_low;
        uint16_t time_moderate;
        uint16_t time_high;
        float total_today;
        float total;
    };

    bool processBufferedData() override;
    bool saveRecords(QVector<PaiRecord> recs);
};

#endif // FETCHPAIOPERATION_H
