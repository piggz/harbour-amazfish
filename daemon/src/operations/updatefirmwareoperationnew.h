#ifndef UPDATEFIRMWAREOPERATIONNEW_H
#define UPDATEFIRMWAREOPERATIONNEW_H

#include "updatefirmwareoperation.h"

class UpdateFirmwareOperationNew : public UpdateFirmwareOperation
{
public:
    UpdateFirmwareOperationNew(const AbstractFirmwareInfo *info, QBLEService *service);

protected:
    bool sendFwInfo() override;
    void sendChecksum() override;
    QByteArray getFirmwareStartCommand() override;
};

#endif // UPDATEFIRMWAREOPERATIONNEW_H
