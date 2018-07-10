#include "sportsdetailoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"
#include "bipactivitydetailparser.h"

SportsDetailOperation::SportsDetailOperation(QBLEService *service, KDbConnection *conn, const ActivitySummary &summary) : AbstractFetchOperation(service)
{
    m_conn = conn;
}

void SportsDetailOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_DETAILS) + rawDate);
    //Send log read command
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
}

void SportsDetailOperation::handleData(const QByteArray &data)
{
    if (data.length() < 2) {
        qDebug() << "unexpected sports summary data length: " << data.length();
        return;
    }

    qDebug() << "Data counter:" << data[0];
    if ((m_lastPacketCounter + 1) == data[0] ) {
        m_lastPacketCounter++;
        if (m_lastPacketCounter > 255) {
            m_lastPacketCounter = 0;
        }
        m_buffer += data.mid(1);
    } else {
        qDebug() << "invalid package counter: " << data[0] << ", last was: " << m_lastPacketCounter;
        finished(false);
        return;
    }
}

bool SportsDetailOperation::finished(bool success)
{
    bool saved = true;
    if (success) {
        BipActivityDetailParser parser(m_summary);
        parser.parse(m_buffer);
        QString gpx = parser.toText();
        qDebug() << gpx;
    }
    return saved;
}

bool SportsDetailOperation::saveDetail()
{
   return true;
}

