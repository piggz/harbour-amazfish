#ifndef FETCHSPO2SLEEPOPERATION_H
#define FETCHSPO2SLEEPOPERATION_H

#include <abstractfetchoperation.h>
#include <KDbConnection.h>

class FetchSpo2SleepOperation : public AbstractFetchOperation
{
public:
    FetchSpo2SleepOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct Spo2SleepRecord {
        QDateTime timestamp;
        uint8_t value;
        uint8_t duration;
        QByteArray spo2High; //6
        QByteArray spo2Low; //8
        QByteArray spo2Quality; //8
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<Spo2SleepRecord> recs);
};

#endif // FETCHSPO2SLEEPOPERATION_H
