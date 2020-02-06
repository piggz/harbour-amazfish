#ifndef BIPLITEDEVICE_H
#define BIPLITEDEVICE_H

#include <QObject>
#include "bipdevice.h"

class BipLiteDevice : public BipDevice
{
public:
    BipLiteDevice(const QString &pairedName, QObject *parent = 0);

    virtual QString deviceType() override;

    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

private:
    void initialise();
    void parseServices();

    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    QString pairedName;

};

#endif // BIPLITEDEVICE_H
