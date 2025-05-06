#ifndef ZEPPOSDEVICE_H
#define ZEPPOSDEVICE_H

#include "zeppos/huami2021chunkeddecoder.h"
#include "zeppos/huami2021chunkedencoder.h"
#include <huamidevice.h>

class AbstractZeppOsService;
class ZeppOsServicesService;
class ZeppOsAuthService;
class ZeppOsNotificationService;
class ZeppOsStepsService;
class ZeppOsHeartRateService;

class ZeppOSDevice: public HuamiDevice, public Huami2021Handler
{
public:
    ZeppOSDevice(const QString &pairedName, QObject *parent = nullptr);

    QString deviceType() const override;
    int supportedFeatures() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    //Overrides from AbstractDevice
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;
    void requestManualHeartrate() const override;


    void writeToChunked2021(short endpoint, QByteArray data, bool encryptIgnored);
    AbstractZeppOsService *zosService(short endpoint) const;
    void addSupportedService(short endpoint, bool encryted);
    void handle2021Payload(short type, const QByteArray &data) override;
    Q_SLOT void authenticated(bool ready) override;
    void ready();
    void setEncryptionParameters(int encryptedSequenceNumber, QByteArray sharedSessionKey);

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list) override;
    void initialise() override;

private:
    QDateTime init_dt = QDateTime::fromTime_t(0);
    void parseServices();

    Huami2021ChunkedEncoder *m_encoder = nullptr;
    Huami2021ChunkedDecoder *m_decoder = nullptr;

    ZeppOsServicesService *m_servicesService = nullptr;
    ZeppOsNotificationService *m_notificationService = nullptr;
    ZeppOsAuthService *m_authService = nullptr;
    ZeppOsStepsService *m_stepsService = nullptr;
    ZeppOsHeartRateService *m_heartRateService = nullptr;

    QMap<short, AbstractZeppOsService *> m_serviceMap;
    QSet<short> m_supportedServices;

    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);
};

#endif // ZEPPOSDEVICE_H
