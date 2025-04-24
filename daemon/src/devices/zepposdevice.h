#ifndef ZEPPOSDEVICE_H
#define ZEPPOSDEVICE_H

#include "zeppos/huami2021chunkeddecoder.h"
#include "zeppos/huami2021chunkedencoder.h"
#include <huamidevice.h>

class AbstractZeppOsService;
class ZeppOsNotificationService;

class ZeppOSDevice: public HuamiDevice
{
public:
    ZeppOSDevice(const QString &pairedName, QObject *parent = nullptr);


    QString deviceType() const override;
    int supportedFeatures() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    void sendAlert(const AbstractDevice::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

    void writeToChunked2021(short endpoint, QByteArray data, bool encryptIgnored);

    AbstractZeppOsService *zosService(short endpoint) const;
    void addSupportedService(short endpoint, bool encryted);

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    virtual void initialise();


private:
    QDateTime init_dt = QDateTime::fromTime_t(0);
    void parseServices();

    Huami2021ChunkedEncoder *m_encoder = nullptr;
    Huami2021ChunkedDecoder *m_decoder = nullptr;

    ZeppOsNotificationService *notificationService = nullptr;

};

#endif // ZEPPOSDEVICE_H
