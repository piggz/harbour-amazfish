#ifndef ABSTRACTOPERATION_H
#define ABSTRACTOPERATION_H

#include "qble/qbleservice.h"

class AbstractOperation
{
public:
    AbstractOperation();
    virtual ~AbstractOperation(){}
    
    virtual void start(QBLEService *service) = 0;

    virtual bool characteristicChanged(const QString &characteristic, const QByteArray &value) = 0;

    bool busy();

    //! Return true if the operation is now complete and can be deleted
    virtual bool handleMetaData(const QByteArray &meta) = 0;
    virtual void handleData(const QByteArray &data) = 0;

protected:
    bool m_busy = false;
    bool m_valid = true;
};

#endif // ABSTRACTOPERATION_H
