#ifndef PINETIMEJFDEVICE_H
#define PINETIMEJFDEVICE_H

#include <QObject>
#include "abstractdevice.h"
#include "realtimeactivitysample.h"

class PinetimeJFDevice : public AbstractDevice
{
public:
    explicit PinetimeJFDevice(const QString &pairedName, QObject *parent = 0);

    void pair() override;
    int supportedFeatures() const override;
    QString deviceType() const override;
    void abortOperations() override;

    void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    void incomingCall(const QString &caller) override;

    void refreshInformation() override;
    QString information(Info i) const override;

    Q_SLOT void authenticated(bool ready);

    void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void startDownload() override;
    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    //Navigation
    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    //Weather
    void sendWeather(CurrentWeather *weather) override;

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    Q_SLOT void serviceEvent(const QString &characteristic, uint8_t event);
    AbstractFirmwareInfo::Type firmwareType;

    RealtimeActivitySample realtimeActivitySample;

    Q_SLOT void sampledActivity(QDateTime dt, int kind, int intensity, int steps, int heartrate);


};

#endif // PINETIMEJFDEVICE_H
