#ifndef GTRDEVICE_H
#define GTRDEVICE_H

#include "gtsdevice.h"
#include <QObject>

class GtrDevice : public GtsDevice
{
public:
    explicit GtrDevice(const QString &pairedName, QObject *parent = nullptr);

private:
    void initialise();
    void parseServices();
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

    bool is47mm(const QString &version) const;
};

#endif // GTRDEVICE_H
