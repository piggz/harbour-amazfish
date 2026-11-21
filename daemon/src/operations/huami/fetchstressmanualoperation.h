#ifndef FETCHSTRESSMANUALOPERATION_H
#define FETCHSTRESSMANUALOPERATION_H

#include <abstractfetchoperation.h>

class FetchStressManualOperation : public AbstractFetchOperation
{
public:
    FetchStressManualOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct StressRecord {
        QDateTime timestamp;
        uint8_t value;
        uint8_t type;
    };
    bool processBufferedData() override;
    bool saveRecords(QVector<StressRecord> recs, QDateTime lastTimeDefault);

};

#endif // FETCHSTRESSAUTOOPERATION_H
