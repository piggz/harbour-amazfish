#ifndef LOGFETCHOPERATION_H
#define LOGFETCHOPERATION_H

#include <QFile>
#include <QDataStream>

#include "abstractfetchoperation.h"

class LogFetchOperation : public AbstractFetchOperation
{
public:
    explicit LogFetchOperation(QBLEService *service);

    void start() override;
    void handleData(const QByteArray &data) override;
    bool finished(bool success) override;

private:
    QFile *m_logFile = nullptr;
    QDataStream *m_dataStream = nullptr;
};

#endif // LOGFETCHOPERATION_H
