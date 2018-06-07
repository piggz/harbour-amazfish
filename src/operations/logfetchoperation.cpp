#include "logfetchoperation.h"
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#include "../mibandservice.h"
#include "../typeconversion.h"

LogFetchOperation::LogFetchOperation(QBLEService *service) : AbstractOperation(service)
{
    QDir cachelocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!cachelocation.exists()) {
        qDebug() << "Creating cahe amazfish folder";
        if (!cachelocation.mkpath("logs")) {
            qDebug() << "Error creating amazfish logs folder!";
            return;
        }
    }

    QString filename = "amazfitbip_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".log";
    m_logFile = new QFile(cachelocation.absolutePath() + "/logs/" + filename);

    if(m_logFile->open(QIODevice::WriteOnly)) {
        m_dataStream = new QDataStream(m_logFile);
    }
}

void LogFetchOperation::start()
{
    QDateTime fetchFrom = QDateTime::currentDateTime();
    fetchFrom.addDays(-10);

    QByteArray rawDate = TypeConversion::dateTimeToBytes(fetchFrom, 0);

    MiBandService *serv = dynamic_cast<MiBandService*>(m_service);

    m_service->enableNotification(serv->UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    m_service->enableNotification(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    m_service->writeValue(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, serv->COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, serv->COMMAND_ACTIVITY_DATA_TYPE_DEBUGLOGS) + rawDate);
    //Send log read command
    m_service->writeValue(serv->UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, serv->COMMAND_FETCH_DATA));
}

bool LogFetchOperation::handleMetaData(const QByteArray &value)
{
    if (value.length() == 15) {
        // first two bytes are whether our request was accepted
        if (value.mid(0, 3) == QByteArray(dynamic_cast<MiBandService*>(m_service)->RESPONSE_ACTIVITY_DATA_START_DATE_SUCCESS, 3)) {
            // the third byte (0x01 on success) = ?
            // the 4th - 7th bytes epresent the number of bytes/packets to expect, excluding the counter bytes
            int expectedDataLength = TypeConversion::toUint32(value[3], value[4], value[5], value[6]);

            // last 8 bytes are the start date
            QDateTime startDate = TypeConversion::rawBytesToDateTime(value.mid(7, 8), false);

            qDebug() << "About to transfer log data from " << startDate;
            m_service->message(QObject::tr("About to transfer log data from ") + startDate.toString());

        } else {
            qDebug() << "Unexpected activity metadata: " << value;
            return true;
        }
    } else if (value.length() == 3) {
        if (value == QByteArray(dynamic_cast<MiBandService*>(m_service)->RESPONSE_FINISH_SUCCESS, 3)) {
            qDebug() << "Finished sending data";
            finished();
            return true;
            m_service->message(QObject::tr("Finished transferring data"));
        } else {
            qDebug() << "Unexpected activity metadata: " << value;
        }
    } else {
        qDebug() << "Unexpected activity metadata: " << value;
    }
    return false;
}

void LogFetchOperation::handleData(const QByteArray &data)
{
    if (m_dataStream) {
        *m_dataStream << data.mid(1);
    }
}

void LogFetchOperation::finished()
{
    if (m_logFile) {
        m_logFile->close();
    }
}
