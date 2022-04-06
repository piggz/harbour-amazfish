#ifndef GTSDEVICE_H
#define GTSDEVICE_H

#include <QObject>
#include "huamidevice.h"

class GtsDevice : public HuamiDevice
{
public:
    explicit GtsDevice(const QString &pairedName, QObject *parent = nullptr);

    QString deviceType() const override;
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event) override;
    virtual int supportedFeatures() const override;
    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    virtual void applyDeviceSetting(Settings s) override;

    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    virtual void sendWeather(CurrentWeather *weather) override;
    virtual void enableFeature(AbstractDevice::Feature feature) override;
    virtual void setMusicStatus(bool playing, const QString &artist, const QString &album, const QString &track, int duration = 0, int position = 0) override;

    //Navigation
    virtual void navigationRunning(bool running) override;
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    virtual QStringList supportedDisplayItems() const override;

protected:
    Q_SLOT void serviceEvent(uint8_t event);
    void initialise() override;
    void parseServices();

private:
    QString pairedName;
    bool m_navigationRunning = false;
};

#endif // GTSDEVICE_H
