#ifndef ASTEROID_OS_DEVICE__H
#define ASTEROID_OS_DEVICE__H

#include "abstractdevice.h"

class AsteroidOSDevice : public AbstractDevice
{
    Q_OBJECT

public:
    explicit AsteroidOSDevice(const QString &pairedName, QObject *parent = nullptr);

    virtual int supportedFeatures() const override;
    virtual QString deviceType() const override;
    virtual void sendAlert(const Amazfish::WatchNotification &notification) override;
    virtual void incomingCall(const QString &caller) override;
    virtual void incomingCallEnded() override;
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;
    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;
    virtual void requestScreenshot() override;

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();

    void pair() override;

    Q_SLOT void authenticated(bool ready);

    void refreshInformation() override;
    QString information(Info i) const override;

    void sendWeather(CurrentWeather *weather) override;

    Q_SLOT void serviceEvent(const QString &characteristic, uint8_t event, uint8_t data);

    Q_SLOT void screenshotReceived(QByteArray data);


    int m_prevVolume = 50;

};

#endif // ASTEROID_OS_DEVICE__H
