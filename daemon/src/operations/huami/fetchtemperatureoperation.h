#ifndef FETCHTEMPERATUREOPERATION_H
#define FETCHTEMPERATUREOPERATION_H

#include <abstractfetchoperation.h>

class FetchTemperatureOperation : public AbstractFetchOperation
{
public:
    FetchTemperatureOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:
    struct TemperatureRecord {
        QDateTime timestamp;
        float value;
        QString type;
        QString location;
    };

    bool processBufferedData() override;
    bool saveRecords(QVector<TemperatureRecord> recs);};

#endif // FETCHTEMPERATUREOPERATION_H
