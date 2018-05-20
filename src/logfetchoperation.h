#ifndef LOGFETCHOPERATION_H
#define LOGFETCHOPERATION_H

#include <QFile>
#include <QDataStream>

class LogFetchOperation
{
public:
    LogFetchOperation();
    void newData(const QByteArray &data);
    void finished();

private:
    QFile *m_logFile = nullptr;
    QDataStream *m_dataStream = nullptr;
};

#endif // LOGFETCHOPERATION_H
