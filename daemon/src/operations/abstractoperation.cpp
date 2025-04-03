#include "abstractoperation.h"

AbstractOperation::AbstractOperation()
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
