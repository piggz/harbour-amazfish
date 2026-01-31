#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "huami/huamifetcher.h"

SportsSummaryOperation::SportsSummaryOperation(HuamiFetcher *fetcher, AbstractActivitySummaryParser *parser, bool isZeppOs) : AbstractFetchOperation(fetcher, isZeppOs), m_parser(parser)
{
    qDebug() << Q_FUNC_INFO;
    setLastSyncKey("device/lastsportsyncmillis");
}

SportsSummaryOperation::~SportsSummaryOperation()
{
    delete m_parser;
}

void SportsSummaryOperation::start(QBLEService *service)
{
    qDebug() << Q_FUNC_INFO;
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    if (!m_parser) {
        qDebug() << "No data parser has been set";
        return;
    }

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, false);
    m_fetcher->setNotifications(true, true);

    //Send read configuration
    QByteArray cmd = QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES) + rawDate;
    m_fetcher->writeControl(cmd);
}

bool SportsSummaryOperation::processBufferedData()
{
    qDebug() << Q_FUNC_INFO << "Buffer:" << m_buffer.toHex();

    if (m_error) {
        qDebug() << Q_FUNC_INFO << "Error detected to skipping process";
        return false;
    }

    m_summary = m_parser->parseBinaryData(m_buffer, startDate());

    if (!m_summary.isValid()) {
        return false;
    }

    return true;
}

bool SportsSummaryOperation::success() const
{
    return !m_error;
}

ActivitySummary SportsSummaryOperation::summary()
{
    return m_summary;
}
