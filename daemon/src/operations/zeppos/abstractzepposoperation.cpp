#include "abstractzepposoperation.h"
#include <qdebug.h>

AbstractZeppOsOperation::AbstractZeppOsOperation()
{

}

void AbstractZeppOsOperation::perform()
{
    doPerform();
}

void AbstractZeppOsOperation::operationFinished()
{
    qDebug() << Q_FUNC_INFO;
}
