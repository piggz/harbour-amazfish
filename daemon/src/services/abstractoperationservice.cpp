#include "abstractoperationservice.h"


AbstractOperationService::AbstractOperationService(const QString &uuid, const QString &path, QObject *parent)
    : QBLEService(uuid, path, parent)
{

}

void AbstractOperationService::registerOperation(AbstractOperation *operation)
{

}
