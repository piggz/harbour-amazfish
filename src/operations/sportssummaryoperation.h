#ifndef ACTIVITYFETCHOPERATION_H
#define ACTIVITYFETCHOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractoperation.h"
#include "activitysample.h"
#include "settingsmanager.h"

class SportsSummaryOperation : public AbstractOperation
{
public:
    SportsSummaryOperation(QBLEService *service, KDbConnection *db);

    void start() override;
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;

    bool finished(bool success);
    void setStartDate(const QDateTime &sd);
private:

    QDateTime m_startDate;
    QDateTime m_sampleTime;
    QList<ActivitySample> m_samples;
    SettingsManager m_settings;

    KDbConnection *m_conn;

    bool save();
    QDateTime lastActivitySync();
};

#endif // ACTIVITYFETCHOPERATION_H
