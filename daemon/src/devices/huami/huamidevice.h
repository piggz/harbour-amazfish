#ifndef HUAMIDEVICE_H
#define HUAMIDEVICE_H

#include "abstractdevice.h"
#include "huami/huamifetcher.h"

#include <QTimer>

class HuamiDevice : public AbstractDevice
{
    Q_OBJECT
public:
    explicit HuamiDevice(const QString &pairedName, QObject *parent = nullptr);
    ~HuamiDevice();
    
    void abortOperations() override;

    void refreshInformation() override;
    QString information(Amazfish::Info i) const override;
    void rebootWatch() override;

    void applyDeviceSetting(Amazfish::Settings s) override;

    virtual void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

    QString softwareRevision();
    
    void downloadSportsData() override;
    void downloadActivityData() override;
    void fetchLogs() override;
    void fetchData(Amazfish::DataTypes dataTypes) override;

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void startDownload() override;

    Q_SLOT virtual void authenticated(bool ready);

    void sendWeatherHuami(CurrentWeather *weather, bool sendConditionString);

    int activitySampleSize();
    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    void setDatabase(KDbConnection *conn) override;

    Q_SLOT void fetchOperationComplete(AbstractFetchOperation *operation);

    AbstractActivitySummaryParser *activitySummaryParser() const override;
    AbstractActivityDetailParser *activityDetailParser() const override;

    virtual void setActivityNotifications(bool control, bool data);
    virtual void writeActivityControl(const QByteArray &value);

    virtual bool isZeppOs() { return false;}

protected:
    Q_SLOT void handleButtonPressed();
    Q_SLOT void buttonPressTimeout();
    Q_SLOT void stepsChanged();
    Q_SLOT void batteryInfoChanged();
    Q_SLOT void serviceEvent(char event);
    Q_SLOT void characteristicChanged(const QString &characteristic, const QByteArray &value);

    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
    virtual void initialise() = 0;

    int m_ActivitySampleSize = 4;
    HuamiFetcher *m_fetcher = nullptr;

private:
    QString m_softwareRevision;
    int m_buttonPresses = 0;
    QTimer *m_keyPressTimer = nullptr;

    QDateTime init_dt = QDateTime::fromTime_t(0);
};

#endif // BIPDEVICE_H
