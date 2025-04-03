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
    ActivityFetchOperation(QBLEService *service, KDbConnection *db, int sampleSize = 4);

    void start(QBLEService *service) override;
    void handleData(const QByteArray &data) override;
    bool finished(bool success) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

private:

    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;

    KDbConnection *m_conn;

    bool saveSamples();
    int m_sampleSize = 4;
};

#endif // ACTIVITYFETCHOPERATION_H
