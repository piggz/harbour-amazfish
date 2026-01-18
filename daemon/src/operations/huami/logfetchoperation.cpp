#include "logfetchoperation.h"
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#include "mibandservice.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"

LogFetchOperation::LogFetchOperation(HuamiFetcher *fetcher, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs)
{
    QDir cachelocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!cachelocation.exists()) {
        qDebug() << Q_FUNC_INFO << "Creating amazfish cache folder";
        if (!cachelocation.mkpath("logs")) {
            qDebug() << Q_FUNC_INFO <<  "Error creating amazfish logs folder!";
            return;
        }
    }

    QString filename = "huami_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".log";
    m_logFile = new QFile(cachelocation.absolutePath() + "/logs/" + filename);

    if(m_logFile->open(QIODevice::WriteOnly)) {
        m_dataStream = new QDataStream(m_logFile);
    }
}

void LogFetchOperation::start(QBLEService *service)
{
    QDateTime fetchFrom = QDateTime::currentDateTime();
    fetchFrom = fetchFrom.addDays(-10);

    QByteArray rawDate = TypeConversion::dateTimeToBytes(fetchFrom, 0);

    m_fetcher->setNotifications(true, true);

    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_DEBUGLOGS) + rawDate;
    //Send log read configuration
    m_fetcher->writeControl(cmd);
    //Send read command
    m_fetcher->writeControl(QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
}

bool LogFetchOperation::processBufferedData()
{
    if (m_dataStream) {
        *m_dataStream << m_buffer;
    }

    if (m_logFile) {
        m_logFile->close();
    }
    return true;
}
