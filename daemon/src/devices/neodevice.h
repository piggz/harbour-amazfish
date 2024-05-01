#ifndef NEODEVICE_H
#define NEODEVICE_H

#include <QObject>
#include "huamidevice.h"

class NeoDevice : public HuamiDevice
{
public:
    explicit NeoDevice(const QString &pairedName, QObject *parent = nullptr);

    QString deviceType() const override;
    void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    int supportedFeatures() const override;
    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void applyDeviceSetting(Settings s) override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    void sendWeather(CurrentWeather *weather) override;

    QStringList supportedDisplayItems() const override;

protected:
    Q_SLOT void serviceEvent(uint8_t event);
    void initialise() override;
    void parseServices();

private:
    QString pairedName;
    bool m_navigationRunning = false;
};

#endif // GTSDEVICE_H
