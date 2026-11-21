
#ifndef DK08_DEVICE__H
#define DK08_DEVICE__H

#include "abstractdevice.h"
#include "activitysample.h"
#include "activitysummary.h"
#include <QObject>

class DK08Device : public AbstractDevice
{
public:
    explicit DK08Device(const QString &pairedName, QObject *parent = 0);

    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;
    void fetchData(Amazfish::DataTypes dataTypes) override;

    void applyDeviceSetting(Amazfish::Settings s) override;
    void sendWeather(CurrentWeather *weather) override;


    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;

    QString information(Amazfish::Info i) const override;

    bool operationRunning() override;

protected:
    void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    QBLEService* drv_createService(const QString &uuid, const QString &path) override;

private:
    void initialise();
    void downloadActivityData();
    void saveActivitySamples(const QList<ActivitySample>& samples, bool complete);
    void saveWorkout(ActivitySummary summary);
    void setOperationRunning(bool running);

    //    Q_SLOT void authenticated(bool ready);

    void refreshInformation() override;

    int m_alert_id = 0;
    bool m_operationRunning = false;
};

#endif // DK08_DEVICE__H
