#include "sportssummaryoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"

SportsSummaryOperation::SportsSummaryOperation(QBLEService *service, KDbConnection *conn, bool isZeppOs, AbstractActivitySummaryParser *parser) : AbstractFetchOperation(isZeppOs), m_parser(parser)
{
    qDebug() << Q_FUNC_INFO;
    m_conn = conn;
    setLastSyncKey("device/lastsportsyncmillis");
}

SportsSummaryOperation::~SportsSummaryOperation()
{
    delete m_parser;
}

void SportsSummaryOperation::start(QBLEService *service)
{
    qDebug() << Q_FUNC_INFO;
    m_service = service;
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    qDebug() << Q_FUNC_INFO << ": Last sync was " << startDate();

    if (!m_parser) {
        qDebug() << "No data parser has been set";
        return;
    }

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate().toUTC(), 0, false);

    service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_SUMMARIES) + rawDate);
}

bool SportsSummaryOperation::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO;
    if (characteristic == MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA) {
        handleData(value);
    } else if (characteristic == MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA) {
        return handleMetaData(value);
    }
    return false;
}

void SportsSummaryOperation::handleData(const QByteArray &data)
{
    qDebug() << Q_FUNC_INFO;
    if (data.length() < 2) {
        qDebug() << Q_FUNC_INFO << "unexpected sports summary data length: " << data.length();
        return;
    }

    qDebug() << Q_FUNC_INFO << "Data counter:" << data[0];
    if ((m_lastPacketCounter + 1) == data[0] ) {
        m_lastPacketCounter++;
        if (m_lastPacketCounter > 255) {
            m_lastPacketCounter = 0;
        }
        m_buffer += data.mid(1);
    } else {
        qDebug() << Q_FUNC_INFO << "invalid package counter: " << data[0] << ", last was: " << m_lastPacketCounter;
        m_error = true;
        return;
    }
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
