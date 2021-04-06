#ifndef SPORTSSUMMARYOPERATION_H
#define SPORTSSUMMARYOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysummary.h"

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
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;
    bool m_success = false;
    ActivitySummary m_summary;

    void parseSummary();
    bool finished(bool success) override;
    QString activityName();

    KDbConnection *m_conn;
};

#endif // SPORTSSUMMARYOPERATION_H
