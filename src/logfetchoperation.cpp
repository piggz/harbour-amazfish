#include "logfetchoperation.h"
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

LogFetchOperation::LogFetchOperation()
{
    QDir cachelocation = QStandardPaths::CacheLocation;
    if (!cachelocation.exists()) {
        qDebug() << "Creating cahe amazfish folder";
        if (!QDir(cachelocation.mkdir("logs"))) {
            qDebug() << "Error creating amazfish logs folder!";
            return;
        }
    }

    QString filename = "amazfitbip_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".log";
    m_logFile = new QFile(cachelocation + "/logs/" + filename);

    if(m_logFile->open(QIODevice::WriteOnly)) {
        m_dataStream = new QDataStream(m_logFile);
    }
}

void LogFetchOperation::newData(const QByteArray &data)
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
