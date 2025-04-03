#ifndef LOGFETCHOPERATION_H
#define LOGFETCHOPERATION_H

#include <QFile>
#include <QDataStream>

#include "abstractfetchoperation.h"

class LogFetchOperation : public AbstractFetchOperation
{
public:
    explicit LogFetchOperation();

    void start(QBLEService *service) override;
    void handleData(const QByteArray &data) override;
    bool finished(bool success) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

private:
    QFile *m_logFile = nullptr;
    QDataStream *m_dataStream = nullptr;
};

#endif // LOGFETCHOPERATION_H
