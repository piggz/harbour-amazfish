#ifndef UPDATEFIRMWAREOPERATION_H
#define UPDATEFIRMWAREOPERATION_H

#include "abstractoperation.h"
#include "abstractdevice.h"

class UpdateFirmwareOperation : public AbstractOperation
{
public:
    UpdateFirmwareOperation(const AbstractFirmwareInfo *info, QBLEService *service, AbstractDevice *device);

    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start(QBLEService *service) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

    QString version();

protected:

    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_fwBytes;

    virtual bool sendFwInfo();
    virtual void sendFirmwareData();
    virtual void sendChecksum();
    virtual QByteArray getFirmwareStartCommand();
    bool m_startWithFWInfo = true;
    QBLEService *m_service = nullptr;
    AbstractDevice *m_device = nullptr;

private:
    bool m_needToSendFwInfo = true;

};

#endif // UPDATEFIRMWAREOPERATION_H
