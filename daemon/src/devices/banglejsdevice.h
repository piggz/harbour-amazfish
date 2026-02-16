#ifndef BANGLEJSDEVICE_H
#define BANGLEJSDEVICE_H

#include <QObject>
#include "abstractdevice.h"
#include "activitysample.h"
#include "activitykind.h"
#include "activitysummary.h"
#include "bangleacttrkrecord.h"

class BangleJSDevice : public AbstractDevice
{
public:
    explicit BangleJSDevice(const QString &pairedName, QObject *parent = 0);

    void pair() override;
    Amazfish::Features supportedFeatures() const override;
    Amazfish::DataTypes supportedDataTypes() const override;
    QString deviceType() const override;
    void abortOperations() override;

    void sendAlert(const Amazfish::WatchNotification &notification) override;
    void incomingCall(const QString &caller) override;
    void incomingCallEnded() override;

    void refreshInformation() override;
    QString information(Amazfish::Info i) const override;

    Q_SLOT void authenticated(bool ready);

    void prepareFirmwareDownload(const AbstractFirmwareInfo *info) override;
    void startDownload() override;
    AbstractFirmwareInfo *firmwareInfo(const QByteArray &bytes, const QString &path) override;

    //Music
    void setMusicStatus(bool playing, const QString &title, const QString &artist, const QString &album, int duration = 0, int position = 0) override;

    //Navigation
    void navigationRunning(bool running) override;
    void navigationNarrative(const QString &flag, const QString &narrative, const QString &manDist, int progress) override;

    void downloadActivityData() override;
    void downloadSportsData() override;

    //Weather
    void sendWeather(CurrentWeather *weather) override;

    void setTime();

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    Q_SLOT void serviceEvent(uint8_t event);
    Q_SLOT void handleRxJson(const QJsonObject &json);

    int m_infoBatteryLevel = 0;
    int m_steps = 0;
    int m_heartrate = 0;
    QString m_firmwareVersion;
    QString m_hardwareVersion;

    QList<ActivitySample> m_samples;
    bool saveActivitySamples();
    ActivityKind::Type convertToActivityKind(const QString &bangle_kind);

    ActivitySummary m_summary;
    void fetchActivityRec(const QString &recId);
    QList<BangleActTrkRecord> m_activityRecords;
    bool saveSportData(const QString& logId);
    QString activityRecordsToText(const QString& logId);
    // QString activityRecordsToTCX(const QString& logId);


};

#endif // BANGLEJSDEVICE_H
