#ifndef GTRDEVICE_H
#define GTRDEVICE_H

#include "gtsdevice.h"
#include <QObject>

class GtrDevice : public GtsDevice
{
public:
    explicit GtrDevice(const QString &pairedName, QObject *parent = nullptr);

protected:
    void initialise() override;
    void parseServices() override;

private:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list) override;

    bool is47mm(const QString &version) const;
};

#endif // GTRDEVICE_H
