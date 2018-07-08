#ifndef LOGFETCHOPERATION_H
#define LOGFETCHOPERATION_H

#include <QFile>
#include <QDataStream>

#include "abstractfetchoperation.h"

class LogFetchOperation : public AbstractFetchOperation
{
public:
    LogFetchOperation(QBLEService *service);

    void start() override;
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void finished();

private:
    QFile *m_logFile = nullptr;
    QDataStream *m_dataStream = nullptr;
};

#endif // LOGFETCHOPERATION_H
