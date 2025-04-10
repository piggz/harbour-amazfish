#ifndef ZEPPOSDEVICE_H
#define ZEPPOSDEVICE_H

#include <huamidevice.h>

class ZeppOSDevice: public HuamiDevice
{
public:
    ZeppOSDevice(const QString &pairedName, QObject *parent = nullptr);


    QString deviceType() const override;
    int supportedFeatures() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    virtual void initialise();


private:
    QDateTime init_dt = QDateTime::fromTime_t(0);
    void parseServices();

};

#endif // ZEPPOSDEVICE_H
