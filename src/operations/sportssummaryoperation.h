#ifndef SPORTSSUMMARYOPERATION_H
#define SPORTSSUMMARYOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractoperation.h"
#include "activitysummary.h"
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
    SettingsManager m_settings;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;

    ActivitySummary *parseSummary();
    bool saveSummary();

    KDbConnection *m_conn;

    bool save();
    QDateTime lastActivitySync();
};

#endif // SPORTSSUMMARYOPERATION_H
