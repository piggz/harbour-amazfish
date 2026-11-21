#ifndef FETCHSPO2SLEEPOPERATION_H
#define FETCHSPO2SLEEPOPERATION_H

#include <abstractfetchoperation.h>

class FetchSpo2SleepOperation : public AbstractFetchOperation
{
public:
    FetchSpo2SleepOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct Spo2SleepRecord {
        QDateTime timestamp;
        uint8_t value;
        uint8_t duration;
        QByteArray spo2High; //6
        QByteArray spo2Low; //6
        QByteArray spo2Quality; //8
    };

    bool processBufferedData() override;
    bool saveRecords(QVector<Spo2SleepRecord> recs);
};

#endif // FETCHSPO2SLEEPOPERATION_H
