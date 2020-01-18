#ifndef UPDATEFIRMWAREOPERATION_H
#define UPDATEFIRMWAREOPERATION_H

#include "abstractoperation.h"
#include "bipfirmwareinfo.h"

class UpdateFirmwareOperation : public AbstractOperation
{
public:
    UpdateFirmwareOperation(const AbstractFirmwareInfo *info, QBLEService *service);

    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start();

    QString version();
private:
    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_fwBytes;

    bool sendFwInfo();
    void sendFirmwareData();
    void sendChecksum();
};

#endif // UPDATEFIRMWAREOPERATION_H
