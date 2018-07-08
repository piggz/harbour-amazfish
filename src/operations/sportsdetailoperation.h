#ifndef SPORTSDETAILOPERATION_H
#define SPORTSDETAILOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysummary.h"
#include "settingsmanager.h"

class SportsDetailOperation : public AbstractFetchOperation
{
public:
    SportsDetailOperation(QBLEService *service, KDbConnection *db, const ActivitySummary &summary);

    void start() override;
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;

private:
    ActivitySummary m_summary;
    SettingsManager m_settings;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;

    ActivitySummary parseDetail();
    bool saveDetail();
    bool finished(bool success);

    KDbConnection *m_conn;

    bool save();
};

#endif // SPORTSSUMMARYOPERATION_H
