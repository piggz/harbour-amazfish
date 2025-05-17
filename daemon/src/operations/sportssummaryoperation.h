#ifndef SPORTSSUMMARYOPERATION_H
#define SPORTSSUMMARYOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractactivitysummaryparser.h"
#include "abstractfetchoperation.h"
#include "activitysummary.h"

class SportsSummaryOperation : public AbstractFetchOperation
{
public:
    SportsSummaryOperation(QBLEService *service, KDbConnection *db, bool isZeppOs = false, AbstractActivitySummaryParser *parser = nullptr);
    ~SportsSummaryOperation();

    void start(QBLEService *service) override;
    void handleData(const QByteArray &data) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

    bool success() const;
    ActivitySummary summary();

private:

    QDateTime m_sampleTime;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;
    bool m_error = false;
    ActivitySummary m_summary;

    bool processBufferedData() override;

    KDbConnection *m_conn;
    AbstractActivitySummaryParser *m_parser = nullptr;
};

#endif // SPORTSSUMMARYOPERATION_H
