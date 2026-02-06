#ifndef FETCHHRVOPERATION_H
#define FETCHHRVOPERATION_H


#include "abstractfetchoperation.h"
#include <KDbConnection.h>

class FetchHrvOperation:  public AbstractFetchOperation
{
public:
    FetchHrvOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct HrvRecord {
        QDateTime timestamp;
        int value;
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<HrvRecord> recs);
};
#endif // FETCHHRVOPERATION_H
