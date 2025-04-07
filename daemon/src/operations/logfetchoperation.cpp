#include "logfetchoperation.h"
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#include "mibandservice.h"
#include "typeconversion.h"

LogFetchOperation::LogFetchOperation()
{
    QDir cachelocation = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (!cachelocation.exists()) {
        qDebug() << Q_FUNC_INFO << "Creating amazfish cache folder";
        if (!cachelocation.mkpath("logs")) {
            qDebug() << Q_FUNC_INFO <<  "Error creating amazfish logs folder!";
            return;
        }
    }

    QString filename = "amazfitbip_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".log";
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

    service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA);
    service->enableNotification(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA);

    //Send log read configuration
    service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_START_DATE) + QByteArray(1, MiBandService::COMMAND_ACTIVITY_DATA_TYPE_DEBUGLOGS) + rawDate);
    //Send log read command
    service->writeValue(MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA, QByteArray(1, MiBandService::COMMAND_FETCH_DATA));
}

void LogFetchOperation::handleData(const QByteArray &data)
{
    if (m_dataStream) {
        *m_dataStream << data.mid(1);
    }
}

bool LogFetchOperation::finished(bool success)
{
    Q_UNUSED(success);
    if (m_logFile) {
        m_logFile->close();
    }
    return true;
}

bool LogFetchOperation::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    if (characteristic == MiBandService::UUID_CHARACTERISTIC_MIBAND_ACTIVITY_DATA) {
        handleData(value);
        return false;
    } else if (characteristic == MiBandService::UUID_CHARACTERISTIC_MIBAND_FETCH_DATA) {
        return handleMetaData(value);
    }
    return false;
}
