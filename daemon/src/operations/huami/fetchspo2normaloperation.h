#ifndef FETCHSPO2NORMALOPERATION_H
#define FETCHSPO2NORMALOPERATION_H

#include "abstractfetchoperation.h"
#include <KDbConnection.h>

class FetchSpo2NormalOperation : public AbstractFetchOperation
{
public:
    FetchSpo2NormalOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct Spo2Record {
        QDateTime timestamp;
        bool automatic;
        uint8_t  value;
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<Spo2Record> recs);
};

#endif // FETCHSPO2NORMALOPERATION_H
