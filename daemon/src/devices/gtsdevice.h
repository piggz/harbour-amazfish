#ifndef GTSDEVICE_H
#define GTSDEVICE_H

#include <QObject>
#include "huamidevice.h"

class GtsDevice : public HuamiDevice
{
public:
    explicit GtsDevice(const QString &pairedName, QObject *parent = nullptr);

    QString deviceType() const override;
    void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    void sendEventReminder(int id, const QDateTime &dt, const QString &event) override;
    int supportedFeatures() const override;
    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void applyDeviceSetting(Settings s) override;

    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    void sendWeather(CurrentWeather *weather) override;
    void enableFeature(AbstractDevice::Feature feature) override;
    void setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration = 0, int position = 0) override;

    //Navigation
    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

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
