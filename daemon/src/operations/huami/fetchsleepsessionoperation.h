#ifndef FETCHSLEEPSESSIONOPERATION_H
#define FETCHSLEEPSESSIONOPERATION_H

#include <KDbConnection.h>
#include <abstractfetchoperation.h>

class FetchSleepSessionOperation : public AbstractFetchOperation
{
public:
    FetchSleepSessionOperation(HuamiFetcher *fetcher, KDbConnection *conn);

    void start(QBLEService *service) override;


private:
    struct SleepSessionRecord {
        QDateTime timestamp;
        QString rawData;
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<SleepSessionRecord> recs);
};

#endif // FETCHSLEEPSESSIONOPERATION_H
