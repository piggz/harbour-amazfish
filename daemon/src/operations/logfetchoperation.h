#ifndef LOGFETCHOPERATION_H
#define LOGFETCHOPERATION_H

#include <QFile>
#include <QDataStream>

#include "abstractfetchoperation.h"

class LogFetchOperation : public AbstractFetchOperation
{
public:
    explicit LogFetchOperation(HuamiFetcher *fetcher, bool isZeppOs = false);

    void start(QBLEService *service) override;

private:
    QFile *m_logFile = nullptr;
    QDataStream *m_dataStream = nullptr;

    bool processBufferedData() override;
};

#endif // LOGFETCHOPERATION_H
