#ifndef EPIXPRO_DEVICE__H
#define EPIXPRO_DEVICE__H

#include <QObject>
#include "garmindevice.h"



class EpixProDevice : public GarminDevice

{
    Q_OBJECT
public:
    explicit EpixProDevice (const QString &pairedName, QObject *parent = 0);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;

};

#endif // EPIXPRO_DEVICE__H
