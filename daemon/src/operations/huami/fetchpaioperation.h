#ifndef FETCHPAIOPERATION_H
#define FETCHPAIOPERATION_H

#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"

class FetchPaiOperation : public AbstractFetchOperation
{
public:
    FetchPaiOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct PaiRecord {
        QDate day;
        float low;
        float moderate;
        float high;
        uint16_t time_low;
        uint16_t time_moderate;
        uint16_t time_high;
        float total_today;
        float total;
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<PaiRecord> recs);
};

#endif // FETCHPAIOPERATION_H
