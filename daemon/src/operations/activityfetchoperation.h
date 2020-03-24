#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysample.h"

class ActivityFetchOperation : public AbstractFetchOperation
{
public:
    ActivityFetchOperation(QBLEService *service, KDbConnection *db);

    void start() override;
    void handleData(const QByteArray &data) override;
    bool finished(bool success) override;

private:

    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;

    KDbConnection *m_conn;

    bool saveSamples();

};

#endif // ACTIVITYFETCHOPERATION_H
