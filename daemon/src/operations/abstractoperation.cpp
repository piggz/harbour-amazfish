#include "abstractoperation.h"

AbstractOperation::AbstractOperation(QBLEService *service) : m_service(service)
{

}

bool AbstractOperation::finished(bool success)
{
    return true;
}
