#ifndef ABSTRACTZEPPOSSERCICE_H
#define ABSTRACTZEPPOSSERCICE_H

#include "zepposdevice.h"
#include <QByteArray>

class AbstractZeppOsService
{

public:
    AbstractZeppOsService(ZeppOSDevice *device);

    virtual void handlePayload(const QByteArray &payload) = 0;
    virtual QString name() const = 0;

    short endpoint() const;
    bool encrypted() const;
    void setEncrypted(bool enc);

protected:
    short m_endpoint = 0x0000;
    bool m_encrypted = false;
    ZeppOSDevice *m_device = nullptr;
};


#endif
