#ifndef BIPLITEDEVICE_H
#define BIPLITEDEVICE_H

#include <QObject>
#include "bipdevice.h"

class BipLiteDevice : public BipDevice
{
public:
    explicit BipLiteDevice(const QString &pairedName, QObject *parent = 0);

    QString deviceType() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

private:
    void initialise() override;
    void parseServices();


private:
    QString pairedName;

};

#endif // BIPLITEDEVICE_H
