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
    void start() override;

    QString version();

protected:

    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_fwBytes;

    virtual bool sendFwInfo();
    virtual void sendFirmwareData();
    virtual void sendChecksum();
    virtual QByteArray getFirmwareStartCommand();
    bool m_startWithFWInfo = true;

private:
    bool m_needToSendFwInfo = true;
};

#endif // UPDATEFIRMWAREOPERATION_H
