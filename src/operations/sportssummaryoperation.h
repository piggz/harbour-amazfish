#ifndef SPORTSSUMMARYOPERATION_H
#define SPORTSSUMMARYOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysummary.h"
#include "settingsmanager.h"

class SportsSummaryOperation : public AbstractFetchOperation
{
public:
    SportsSummaryOperation(QBLEService *service, KDbConnection *db);

    void start() override;
    void handleData(const QByteArray &data) override;

    bool success() const;
    ActivitySummary summary();

private:

    QDateTime m_sampleTime;
    SettingsManager m_settings;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;
    bool m_success = false;
    ActivitySummary m_summary;

    ActivitySummary parseSummary();
    bool saveSummary();
    bool finished(bool success);
    QString activityName();

    KDbConnection *m_conn;
};

#endif // SPORTSSUMMARYOPERATION_H
