#ifndef UPDATEFIRMWAREOPERATION_H
#define UPDATEFIRMWAREOPERATION_H

#include "abstractoperation.h"

class UpdateFirmwareOperation : public AbstractOperation
{
public:
    UpdateFirmwareOperation(QBLEService *service);

    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start();
};

#endif // UPDATEFIRMWAREOPERATION_H
