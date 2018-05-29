#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "activitysample.h"
#include "settingsmanager.h"

class ActivityFetchOperation
{
public:
    ActivityFetchOperation(const QDateTime &sd, KDbConnection *db);

    void newData(const QByteArray &data);
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
};

#endif // ACTIVITYFETCHOPERATION_H
