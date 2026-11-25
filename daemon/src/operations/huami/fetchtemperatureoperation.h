#ifndef FETCHTEMPERATUREOPERATION_H
#define FETCHTEMPERATUREOPERATION_H

#include "database.h"
#include <abstractfetchoperation.h>

class FetchTemperatureOperation : public AbstractFetchOperation
{
public:
    FetchTemperatureOperation(HuamiFetcher* fetcher, bool isZeppOs = false);

    void start() override;

private:

    bool processBufferedData() override;
    bool saveRecords(QVector<Database::TemperatureRecord> recs);
};

#endif // FETCHTEMPERATUREOPERATION_H
