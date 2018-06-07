#ifndef UPDATEFIRMWAREOPERATION_H
#define UPDATEFIRMWAREOPERATION_H

#include "abstractoperation.h"

class UpdateFirmwareOperation : public AbstractOperation
{
public:
    UpdateFirmwareOperation();

    virtual bool handleMetaData(const QByteArray &meta) override;
    virtual void handleData(const QByteArray &data) override;
    virtual void start();
};

#endif // UPDATEFIRMWAREOPERATION_H
