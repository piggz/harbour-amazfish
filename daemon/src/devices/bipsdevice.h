#ifndef BIPSDEVICE_H
#define BIPSDEVICE_H

#include "bipdevice.h"

class BipSDevice : public BipDevice
{
    Q_OBJECT
public:
    BipSDevice(const QString &pairedName, QObject *parent = nullptr);

    virtual QString deviceType() override;
    virtual int supportedFeatures() override;
    virtual QStringList supportedDisplayItems() const override;

    virtual void applyDeviceSetting(Settings s) override;

    AbstractFirmwareInfo* firmwareInfo(const QByteArray &bytes) override;

    virtual void setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration = 0, int position = 0) override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list) override;

private:
    Q_SLOT void serviceEvent(uint8_t event);

    void parseServices();
    void initialise();
};

#endif // BIPSDEVICE_H
