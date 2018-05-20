#include "logfetchoperation.h"
#include <QDir>
#include <QDateTime>
#include <QDebug>

LogFetchOperation::LogFetchOperation()
{
    if (!QDir(QDir::homePath() + "/amazfish").exists()) {
        qDebug() << "Creating amazfish folder";
        if (!QDir(QDir::homePath()).mkdir("amazfish")) {
            qDebug() << "Error creating amazfish folder!";
            return;
        }
    }

    QString filename = "amazfitbip_" + QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss") + ".log";
    m_logFile = new QFile(QDir::homePath() + "/amazfish/" + filename);

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
