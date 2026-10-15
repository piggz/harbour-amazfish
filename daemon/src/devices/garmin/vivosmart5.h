#ifndef VIVOSMART5_H
#define VIVOSMART5_H

#include "garmindevice.h"
#include <QObject>

class Vivosmart5Device : public GarminDevice
{
    Q_OBJECT
public:
    explicit Vivosmart5Device (const QString &pairedName, QObject *parent = 0);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
};

#endif // VIVOSMART5_H
