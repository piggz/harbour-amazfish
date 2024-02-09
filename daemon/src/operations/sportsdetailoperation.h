#ifndef SPORTSDETAILOPERATION_H
#define SPORTSDETAILOPERATION_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <KDb3/KDbConnection>

#include "abstractfetchoperation.h"
#include "activitysummary.h"

class SportsDetailOperation : public AbstractFetchOperation
{
public:
    SportsDetailOperation(QBLEService *service, KDbConnection *db, const ActivitySummary &summary);

    void start() override;
    void handleData(const QByteArray &data) override;

private:
    ActivitySummary m_summary;
    int m_lastPacketCounter = 0;
    QByteArray m_buffer;
    QString m_gpx;

    bool saveSport(const QString &filename);
    bool finished(bool success) override;

    KDbConnection *m_conn;

    bool save();
};

#endif // SPORTSDETAILOPERATION_H
