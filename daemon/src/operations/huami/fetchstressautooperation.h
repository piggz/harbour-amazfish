#ifndef FETCHSTRESSAUTOOPERATION_H
#define FETCHSTRESSAUTOOPERATION_H

#include <abstractfetchoperation.h>

class FetchStressAutoOperation : public AbstractFetchOperation
{
public:
    FetchStressAutoOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

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
