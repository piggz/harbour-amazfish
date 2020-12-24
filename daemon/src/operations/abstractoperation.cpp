#include "abstractoperation.h"

AbstractOperation::AbstractOperation(QBLEService *service) : m_service(service)
{

}

bool AbstractOperation::finished(bool success)
{
    Q_UNUSED(success);
    return true;
}

bool AbstractOperation::busy()
{
    return m_busy;
}
