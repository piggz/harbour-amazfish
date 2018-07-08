#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysample.h"
#include "settingsmanager.h"

class ActivityFetchOperation : public AbstractFetchOperation
{
public:
    ActivityFetchOperation(QBLEService *service, KDbConnection *db);

    void start() override;
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;

    bool finished(bool success);
private:

    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;
    SettingsManager m_settings;

    KDbConnection *m_conn;

    bool saveSamples();

};

#endif // ACTIVITYFETCHOPERATION_H
