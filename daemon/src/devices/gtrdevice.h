#ifndef GTRDEVICE_H
#define GTRDEVICE_H

#include "gtsdevice.h"
#include <QObject>

class GtrDevice : public GtsDevice
{
public:
    explicit GtrDevice(const QString &pairedName, QObject *parent = nullptr);
    QString deviceType() const override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

protected:
    void initialise() override;
    void parseServices();

private:
    bool is47mm(const QString &version) const;
};

#endif // GTRDEVICE_H
