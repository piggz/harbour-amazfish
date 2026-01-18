
#ifndef DK08_DEVICE__H
#define DK08_DEVICE__H

#include <QObject>
#include "abstractdevice.h"

class DK08Device : public AbstractDevice
{
public:
    explicit DK08Device(const QString &pairedName, QObject *parent = 0);

    int supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;
    void downloadActivityData() override;
    void applyDeviceSetting(Settings s) override;
    void sendWeather(CurrentWeather *weather) override;


    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;

    QString information(Info i) const override;

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    void pair() override;


//    Q_SLOT void authenticated(bool ready);

    void refreshInformation() override;

    int m_alert_id = 0;

};

#endif // DK08_DEVICE__H
