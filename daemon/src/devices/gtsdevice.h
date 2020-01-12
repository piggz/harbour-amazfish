#ifndef GTSDEVICE_H
#define GTSDEVICE_H

#include <QObject>
#include "bipdevice.h"

class GtsDevice : public BipDevice
{
public:
    GtsDevice();
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;

private:
    void initialise();
    void parseServices();

    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

};

#endif // GTSDEVICE_H
