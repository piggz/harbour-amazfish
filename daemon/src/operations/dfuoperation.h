#ifndef DFUOPERATION_H
#define DFUOPERATION_H

#include "abstractoperation.h"
#include "bipfirmwareinfo.h"
#include "dfuworker.h"

class DfuOperation : public QObject, public AbstractOperation
{
    Q_OBJECT
public:
    DfuOperation(const AbstractFirmwareInfo *info, QBLEService *service);
    ~DfuOperation();
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start() override;

protected:

    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_fwBytes;

    virtual bool sendFwInfo();
    virtual void sendFirmwareData();

private:
    int m_outstandingPacketNotifications = 0;
    int m_notificationPackets = 40;
    bool m_transferError = false;
    QThread m_workerThread;
    DfuWorker *m_worker;

    Q_SIGNAL void sendFirmware(DfuService* service, const QByteArray &data, int notificationPackets);
    Q_SLOT void packetNotification();
};

#endif // DFUOPERATION_H
