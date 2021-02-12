#ifndef BIPSDEVICE_H
#define BIPSDEVICE_H

#include "bipdevice.h"

class BipSDevice : public BipDevice
{
    Q_OBJECT
public:
    BipSDevice(const QString &pairedName, QObject *parent = nullptr);

    virtual QString deviceType() override;
    virtual void applyDeviceSetting(Settings s) override;

    AbstractFirmwareInfo* firmwareInfo(const QByteArray &bytes) override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list) override;

private:
    void parseServices();
    void initialise();
};

#endif // BIPSDEVICE_H
