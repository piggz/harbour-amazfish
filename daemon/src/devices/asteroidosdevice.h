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
    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void incomingCall(const QString &caller) override;
    virtual void incomingCallEnded() override;
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;
    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;
    virtual void requestScreenshot() override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();

    virtual void pair() override;

    Q_SLOT void authenticated(bool ready);

    virtual void refreshInformation() override;
    virtual void sendWeather(CurrentWeather *weather) override;

    Q_SLOT void serviceEvent(const QString &characteristic, uint8_t event, uint8_t data);

    Q_SLOT void screenshotReceived(QByteArray data);


    int m_prevVolume = 50;


/*

    virtual void connectToDevice() override;
    virtual void disconnectFromDevice() override;
    virtual QString connectionState() const;

    QString deviceName() const;
    virtual void abortOperations();

    //Firmware handling
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) = 0; //Caller owns the pointer and should delete it
    virtual void prepareFirmwareDownload(const AbstractFirmwareInfo* info);
    virtual void startDownload();

    virtual void downloadSportsData();
    virtual QString information(Info i) const;
    virtual void applyDeviceSetting(Settings s);
    virtual void rebootWatch();
    virtual void sendEventReminder(int id, const QDateTime &dt, const QString &event);
    virtual void enableFeature(AbstractDevice::Feature feature);
    virtual void navigationRunning(bool running);
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress);
    virtual QStringList supportedDisplayItems() const;
*/
};

#endif // ASTEROID_OS_DEVICE__H
