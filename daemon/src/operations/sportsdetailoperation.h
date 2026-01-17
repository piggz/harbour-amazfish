#ifndef SPORTSDETAILOPERATION_H
#define SPORTSDETAILOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractactivitydetailparser.h"
#include "abstractfetchoperation.h"
#include "activitysummary.h"

class SportsDetailOperation : public AbstractFetchOperation
{
public:
    SportsDetailOperation(HuamiFetcher *fetcher, KDbConnection *db, const ActivitySummary &summary, AbstractActivityDetailParser *parser = nullptr, bool isZeppOs = false);
    ~SportsDetailOperation();

    void start(QBLEService *service) override;
    void handleData(const QByteArray &data) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

private:
    ActivitySummary m_summary;
    AbstractActivityDetailParser *m_parser = 0;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;
    QString m_gpx;

    bool saveSport(const QString &filename);

    KDbConnection *m_conn;

    bool save();

    bool processBufferedData() override;

};

#endif // SPORTSDETAILOPERATION_H
