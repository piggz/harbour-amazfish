#ifndef GTR2DEVICE_H
#define GTR2DEVICE_H

#include "gts2device.h"
#include <QObject>

class Gtr2Device : public Gts2Device
{
public:
    explicit Gtr2Device(const QString &pairedName, QObject *parent = nullptr);
    QString deviceType() const override;
    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

};

#endif // GTR2DEVICE_H
