
#ifndef PEBBLE_DEVICE__H
#define PEBBLE_DEVICE__H

#include <QObject>
#include "abstractdevice.h"

class PebbleDevice : public AbstractDevice
{
public:
    explicit PebbleDevice(const QString &pairedName, QObject *parent = 0);

    virtual int supportedFeatures() const override;
    virtual QString deviceType() const override;
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void incomingCall(const QString &caller) override;
    virtual void incomingCallEnded() override;

   virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

   QString information(Info i) const override;


protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    virtual void pair() override;


//    Q_SLOT void authenticated(bool ready);

   virtual void refreshInformation() override;

};

#endif // PEBBLE_DEVICE__H
