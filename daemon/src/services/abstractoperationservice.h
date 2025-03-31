#ifndef ABSTRACTOPERATIONSERVICE_H
#define ABSTRACTOPERATIONSERVICE_H

#include "abstractoperation.h"

#include <qbleservice.h>


class AbstractOperationService : public QBLEService
{
    Q_OBJECT
public:
    explicit AbstractOperationService(const QString &uuid, const QString &path, QObject *parent);

    void registerOperation(AbstractOperation *operation);

private:

    QList<AbstractOperation*> m_operations;
};

#endif // ABSTRACTOPERATIONSERVICE_H
