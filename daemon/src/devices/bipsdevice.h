#ifndef BIPSDEVICE_H
#define BIPSDEVICE_H

#include "bipdevice.h"

class BipSDevice : public BipDevice
{
    Q_OBJECT
public:
    BipSDevice(const QString &pairedName, QObject *parent = nullptr);

    virtual QString deviceType() override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list) override;

private:
    void parseServices();
    void initialise();
};

#endif // BIPSDEVICE_H
