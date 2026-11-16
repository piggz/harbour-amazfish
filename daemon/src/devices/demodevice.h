#ifndef DEMODEVICE_H
#define DEMODEVICE_H

#include "abstractdevice.h"

// A watch without Bluetooth, for screenshots and UI tests. It supports every feature, connects at once
// and fills an empty database with one week of sample data. Built with the CMake option DEMO_DEVICE.
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
    void setDatabase(KDbConnection* conn) override;
    void fetchData(Amazfish::DataTypes dataTypes) override;
    void refreshInformation() override;
    QString information(Amazfish::Info i) const override;
    QStringList supportedDisplayItems() const override;

protected:
    QBLEService* drv_createService(const QString& uuid, const QString& path) override;

private:
    void addSampleData();
    bool insert(const QString& table, const QStringList& fieldNames, const QList<QVariant>& values);
};

#endif // DEMODEVICE_H
