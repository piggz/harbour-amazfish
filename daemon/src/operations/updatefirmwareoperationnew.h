#ifndef UPDATEFIRMWAREOPERATIONNEW_H
#define UPDATEFIRMWAREOPERATIONNEW_H

#include "updatefirmwareoperation.h"

class UpdateFirmwareOperationNew : public UpdateFirmwareOperation
{
public:
    UpdateFirmwareOperationNew(const AbstractFirmwareInfo *info, QBLEService *service);
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override {return false;};

protected:
    bool sendFwInfo() override;
    void sendChecksum() override;
    QByteArray getFirmwareStartCommand() override;
};

#endif // UPDATEFIRMWAREOPERATIONNEW_H
