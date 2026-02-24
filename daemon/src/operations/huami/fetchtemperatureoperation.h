#ifndef FETCHTEMPERATUREOPERATION_H
#define FETCHTEMPERATUREOPERATION_H

#include <abstractfetchoperation.h>
#include <KDbConnection.h>

class FetchTemperatureOperation : public AbstractFetchOperation
{
public:
    FetchTemperatureOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    struct TemperatureRecord {
        QDateTime timestamp;
        float value;
        QString type;
        QString location;
    };

    KDbConnection *m_conn;

    bool processBufferedData() override;
    bool saveRecords(QVector<TemperatureRecord> recs);};

#endif // FETCHTEMPERATUREOPERATION_H
