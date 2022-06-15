#ifndef GTS2DEVICE_H
#define GTS2DEVICE_H

#include "gtsdevice.h"

/*
 *{0000FEE1-0000-1000-8000-00805f9b34fb} MiBand2 Service
--00000009-0000-3512-2118-0009af100700 //Auth
--00000017-0000-3512-2118-0009af100700 //2021 Chunked Char Read
*/
class Gts2Device : public GtsDevice
{
    Q_OBJECT
public:
    explicit Gts2Device(const QString &pairedName, QObject *parent = nullptr);
    QString deviceType() const override;
    QStringList supportedDisplayItems() const override;
    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;
    void incomingCall(const QString &caller) override;

protected:
    void initialise() override;
};

#endif // GTS2DEVICE_H
