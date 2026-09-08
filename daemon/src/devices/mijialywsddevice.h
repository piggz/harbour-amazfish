#ifndef MIJIALYWSDDEVICE_H
#define MIJIALYWSDDEVICE_H

#include "abstractdevice.h"

class MijiaLywsdService;

class MijiaLywsdDevice : public AbstractDevice
{
    Q_OBJECT
public:
    explicit MijiaLywsdDevice(const QString &pairedName, QObject *parent = nullptr);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;

    void pair() override;

    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

    QString deviceType() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &fileName) override;

    void refreshInformation() override;
    
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

protected:
    QBLEService* drv_createService(const QString &uuid, const QString &path) override;

private:
    void initialise();

};

#endif // MIJIALYWSDDEVICE_H
