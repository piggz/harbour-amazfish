#ifndef BIPSDEVICE_H
#define BIPSDEVICE_H

#include "bipdevice.h"

class BipSDevice : public BipDevice
{
    Q_OBJECT
public:
    explicit BipSDevice(const QString &pairedName, QObject *parent = nullptr);

    QString deviceType() const override;
    int supportedFeatures() const override;
    QStringList supportedDisplayItems() const override;

    void applyDeviceSetting(Settings s) override;

    AbstractFirmwareInfo* firmwareInfo(const QByteArray &bytes) override;

    void setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration = 0, int position = 0) override;

protected:
    void initialise() override;

private:
    Q_SLOT void serviceEvent(uint8_t event);
    void parseServices();
};

#endif // BIPSDEVICE_H
