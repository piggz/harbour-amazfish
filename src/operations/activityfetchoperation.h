#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractoperation.h"
#include "activitysample.h"
#include "settingsmanager.h"

class ActivityFetchOperation : public AbstractOperation
{
public:
    ActivityFetchOperation(QBLEService *service, KDbConnection *db);

    void start() override;
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;

    bool finished(bool success);
    void setStartDate(const QDateTime &sd);
private:

    QFile *m_logFile = nullptr;
    QTextStream *m_dataStream = nullptr;
    QDateTime m_startDate;
    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;
    SettingsManager m_settings;

    KDbConnection *m_conn;

    bool saveSamples();
    QDateTime lastActivitySync();
};

#endif // ACTIVITYFETCHOPERATION_H
