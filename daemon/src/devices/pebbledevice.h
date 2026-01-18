
#ifndef PEBBLE_DEVICE__H
#define PEBBLE_DEVICE__H

#include <QObject>
#include "abstractdevice.h"

class PebbleDevice : public AbstractDevice
{
public:
    explicit PebbleDevice(const QString &pairedName, QObject *parent = 0);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;

    QString information(Amazfish::Info i) const override;


protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    virtual void pair() override;


//    Q_SLOT void authenticated(bool ready);

   virtual void refreshInformation() override;

};

#endif // PEBBLE_DEVICE__H
