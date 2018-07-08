#include "sportsdetailoperation.h"

#include <QDebug>
#include <QDataStream>
#include <KDb3/KDbTransactionGuard>

#include "mibandservice.h"
#include "typeconversion.h"
#include "activitykind.h"

SportsDetailOperation::SportsDetailOperation(QBLEService *service, KDbConnection *conn, const ActivitySummary &summary) : AbstractFetchOperation(service)
{
    m_conn = conn;
}

void SportsDetailOperation::start()
{
    setStartDate(lastActivitySync());
    m_lastPacketCounter = -1;

    QByteArray rawDate = TypeConversion::dateTimeToBytes(startDate(), 0);

    MiBandService *serv = dynamic_cast<MiBandService*>(m_service);

    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_SPORTS_DETAILS) + rawDate);
    //Send log read command
    m_service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
}

bool SportsDetailOperation::handleMetaData(const QByteArray &value)
{
    if (value.length() == 15) {
        // first two bytes are whether our request was accepted
        if (value.mid(0, 3) == QByteArray(MiBandService::RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS, 3)) {
            // the third byte (0x01 on success) = ?
            // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
            int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

            // last 8 bytes are the start date
            QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), false);
            setStartDate(startDate);

            qDebug() << "About to transfer activity data from " << startDate;
            m_service->message(QObject::tr("About to transfer activity data from ") + startDate.toString());

        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else if (value.length() == 3) {
        if (value == QByteArray(dynamic_cast<MiBandService*>(m_service)->RESPONSE_FINISH_SUCCESS, 3)) {
            qDebug() << "Finished sending data";
            finished(true);
            m_service->message(QObject::tr("Finished transferring activity data"));
            return true;
        } else if (value == QByteArray(dynamic_cast<MiBandService*>(m_service)-> RESPONSE_FINISH_FAIL, 3)) {
            qDebug() << "No data left to fetch";
            m_service->message(QObject::tr("No data to transfer"));
            return true;
        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else {
        qDebug() << "Unexpected activity metadata: " << value;
    }
    return false;
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
//        ActivitySummary summary = parseDetail();
//        saved = saveSummary(summary);
    }
    return saved;
}

ActivitySummary SportsDetailOperation::parseDetail()
{

}

bool SportsDetailOperation::saveDetail()
{
   return true;
}

