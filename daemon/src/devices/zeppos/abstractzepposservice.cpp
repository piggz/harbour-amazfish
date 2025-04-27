#include "abstractzepposservice.h"

short AbstractZeppOsService::endpoint() const
{
    return m_endpoint;
}

bool AbstractZeppOsService::encrypted() const
{
    return m_encrypted;
}

void AbstractZeppOsService::setEncrypted(bool enc)
{
    qDebug() << "Setting service " << m_endpoint << " encrypted " << enc;
    m_encrypted = enc;
}

AbstractZeppOsService::AbstractZeppOsService(ZeppOSDevice *device, bool encryptedDefault) : m_device(device), m_encrypted(encryptedDefault)
{

}
