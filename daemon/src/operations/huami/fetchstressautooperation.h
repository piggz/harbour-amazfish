#ifndef FETCHSTRESSAUTOOPERATION_H
#define FETCHSTRESSAUTOOPERATION_H

#include <abstractfetchoperation.h>
#include <KDbConnection.h>

class FetchStressAutoOperation : public AbstractFetchOperation
{
public:
    FetchStressAutoOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct StressRecord {
        QDateTime timestamp;
        uint8_t value;
        uint8_t type;
    };
    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<StressRecord> recs, QDateTime lastTimeDefault);

};

#endif // FETCHSTRESSAUTOOPERATION_H
