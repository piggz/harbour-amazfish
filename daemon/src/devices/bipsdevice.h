#ifndef BIPSDEVICE_H
#define BIPSDEVICE_H

#include "huamidevice.h"

class BipSDevice : public HuamiDevice
{
    Q_OBJECT
public:
    BipSDevice(const QString &pairedName, QObject *parent = nullptr);

    virtual QString deviceType() override;

    virtual int supportedFeatures() override;
    virtual QStringList supportedDisplayItems() const override;
    virtual void applyDeviceSetting(AbstractDevice::Settings s) override;

    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    virtual void sendWeather(CurrentWeather *weather) override;

    virtual void navigationRunning(bool running) override;
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
};

#endif // BIPSDEVICE_H
