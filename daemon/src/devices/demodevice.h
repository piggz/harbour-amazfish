#ifndef DEMODEVICE_H
#define DEMODEVICE_H

#include "abstractdevice.h"

// A watch without Bluetooth, for screenshots and UI tests. It supports every feature and connects at once.
// Used when the daemon runs with AMAZFISH_DEMO_DEVICE=1.
class DemoDevice : public AbstractDevice {
public:
    explicit DemoDevice(const QString& pairedName, QObject* parent = nullptr);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    AbstractFirmwareInfo* firmwareInfo(const QByteArray& bytes, const QString& fileName) override;
    void sendAlert(const Amazfish::WatchNotification& notification) override;
    void incomingCall(const QString& caller) override;
    void incomingCallEnded() override;

    void connectToDevice() override;
    void disconnectFromDevice() override;
    void fetchData(Amazfish::DataTypes dataTypes) override;
    void refreshInformation() override;
    QString information(Amazfish::Info i) const override;
    QStringList supportedDisplayItems() const override;

protected:
    QBLEService* drv_createService(const QString& uuid, const QString& path) override;
};

#endif // DEMODEVICE_H
