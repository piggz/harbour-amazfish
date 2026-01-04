#ifndef ABSTRACTZEPPOSSERCICE_H
#define ABSTRACTZEPPOSSERCICE_H

#include <QByteArray>
#include <QDebug>

class ZeppOSDevice;

class AbstractZeppOsService
{

public:
    AbstractZeppOsService(ZeppOSDevice *device, bool encryptedDefault);

    virtual void handlePayload(const QByteArray &payload) = 0;
    virtual QString name() const = 0;

    short endpoint() const;
    bool encrypted() const;
    void setEncrypted(bool enc);

    virtual void initialize();

protected:
    short m_endpoint = 0x0000;
    bool m_encrypted = false;
    ZeppOSDevice *m_device = nullptr;

    void write(const QByteArray &data);
};


#endif
