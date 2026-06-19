#ifndef FETCHSLEEPSESSIONOPERATION_H
#define FETCHSLEEPSESSIONOPERATION_H

#include <KDbConnection.h>
#include <abstractfetchoperation.h>

class FetchSleepSessionOperation : public AbstractFetchOperation
{
public:
    FetchSleepSessionOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;


private:
    struct SleepSessionRecord {

    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<SleepSessionRecord> recs);
};

#endif // FETCHSLEEPSESSIONOPERATION_H
