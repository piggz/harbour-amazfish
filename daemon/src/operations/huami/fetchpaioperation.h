#ifndef FETCHPAIOPERATION_H
#define FETCHPAIOPERATION_H

#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysample.h"

#include <abstractfetchoperation.h>

class FetchPaiOperation : public AbstractFetchOperation
{
public:
    FetchPaiOperation(HuamiFetcher *fetcher, KDbConnection *conn, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:

    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;

    KDbConnection *m_conn;

    bool processBufferedData() override;
};

#endif // FETCHPAIOPERATION_H
