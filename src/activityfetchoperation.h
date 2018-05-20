#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "activitysample.h"
#include "settingsmanager.h"

class ActivityFetchOperation
{
public:
    ActivityFetchOperation(const QDateTime &sd);

    void newData(const QByteArray &data);
    void finished(bool success);
    void setStartDate(const QDateTime &sd);
private:

    QFile *m_logFile = nullptr;
    QTextStream *m_dataStream = nullptr;
    QDateTime m_startDate;
    QList<ActivitySample> m_samples;
    SettingsManager m_settings;

    void saveSamples();
};

#endif // ACTIVITYFETCHOPERATION_H
