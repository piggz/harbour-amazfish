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

private:
    ActivitySummary m_summary;
    AbstractActivityDetailParser *m_parser = 0;
    QString m_gpx;
    KDbConnection *m_conn;

    bool saveSport(const QString &filename);
    bool save();
    bool processBufferedData() override;
};

#endif // SPORTSDETAILOPERATION_H
