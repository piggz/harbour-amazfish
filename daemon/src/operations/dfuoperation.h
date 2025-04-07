#ifndef DFUOPERATION_H
#define DFUOPERATION_H

#include "abstractdevice.h"
#include "abstractoperation.h"
#include "dfuworker.h"

class DfuService;
class AbstractFirmwareInfo;
class DfuOperation : public QObject, public AbstractOperation
{
    Q_OBJECT
public:
    DfuOperation(const AbstractFirmwareInfo *info, QBLEService *service, AbstractDevice *device);
    ~DfuOperation();
    bool handleMetaData(const QByteArray &meta) override;
    void handleData(const QByteArray &data) override;
    void start(QBLEService *service) override;
    bool characteristicChanged(const QString &characteristic, const QByteArray &value) override;

protected:
    const AbstractFirmwareInfo *m_info = nullptr;
    QByteArray m_uncompressedFwBytes;

    virtual bool sendFwInfo();
    virtual void sendFirmwareData();

private:
    int m_outstandingPacketNotifications = 0;
    int m_notificationPackets = 10;
    bool m_transferError = false;
    QThread m_workerThread;
    DfuWorker *m_worker = nullptr;
    uint16_t m_crc16;

    Q_SIGNAL void sendFirmware(DfuService* service, const QByteArray &data, int notificationPackets);

    Q_SLOT void packetNotification();
    bool probeArchive();
    QBLEService *m_service = nullptr;
    AbstractDevice *m_device = nullptr;
    };

#endif // DFUOPERATION_H
