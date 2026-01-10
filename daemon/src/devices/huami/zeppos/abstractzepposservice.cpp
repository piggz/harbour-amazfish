#include "abstractzepposservice.h"
#include "huami/zepposdevice.h"

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

void AbstractZeppOsService::initialize()
{

}

void AbstractZeppOsService::write(const QByteArray &data)
{
    m_device->writeToChunked2021(endpoint(), data, encrypted());
}

AbstractZeppOsService::AbstractZeppOsService(ZeppOSDevice *device, bool encryptedDefault) : m_device(device), m_encrypted(encryptedDefault)
{

}
