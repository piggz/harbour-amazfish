#include "abstractoperation.h"

AbstractOperation::AbstractOperation()
{

}

bool AbstractOperation::busy()
{
    return m_busy;
}
