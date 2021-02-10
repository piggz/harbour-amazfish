#ifndef BANGLEJSDEVICE_H
#define BANGLEJSDEVICE_H

#include <QObject>
#include "abstractdevice.h"

class BangleJSDevice : public AbstractDevice
{
public:
    BangleJSDevice(const QString &pairedName, QObject *parent = 0);

    virtual QString pair() override;
    virtual int supportedFeatures() override;
    virtual QString deviceType() override;
    virtual bool operationRunning() override;
    virtual void abortOperations() override;

    virtual void sendAlert(const QString &sender, const QString &subject, const QString &message) override;
    virtual void incomingCall(const QString &caller) override;

    virtual void refreshInformation() override;
    virtual QString information(Info i) const override;

    Q_SLOT void authenticated(bool ready);

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    virtual void startDownload() override;
    virtual AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes) override;

    //Music
    virtual void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;

    //Navigation
    virtual void navigationRunning(bool running) override;
    virtual void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    //Weather
    virtual void sendWeather(CurrentWeather *weather) override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    Q_SLOT void serviceEvent(uint8_t event);
    Q_SLOT void handleRxJson(const QJsonObject &json);

    int m_infoBatteryLevel = 0;
    QString m_firmwareVersion;
};

#endif // BANGLEJSDEVICE_H
