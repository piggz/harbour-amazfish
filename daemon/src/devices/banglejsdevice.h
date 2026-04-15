#ifndef BANGLEJSDEVICE_H
#define BANGLEJSDEVICE_H

#include <optional>
#include <map>
#include <QObject>
#include "abstractdevice.h"
#include "activitysample.h"
#include "activitykind.h"
#include "activitysummary.h"
#include "bangleacttrkrecord.h"

class BangleJSDevice : public AbstractDevice
{
public:
    static inline std::map<QString, QString> AppToIconMap = {
        // {"ubuntu/SFOS name", "bangle name"}
        // Sources:
        // - https://github.com/espruino/BangleApps/blob/master/apps/messageicons/icons/icon_names.json
        // - https://open-store.io  (Ubuntu Touch app IDs)
        // - https://sailfishos-chum.github.io  (SailfishOS Chum packages)

        {"Lomiri Telephony Service Indicator", "sms message"}, // UT: SMS
        {"dekko2.dekkoproject_dekko", "mail"},  // UT: Email client
        {"teleports.ubports", "telegram"},      // UT: TELEports
        {"teleports.ubports_teleports", "telegram"}, // UT: TELEports
        {"depecher", "telegram"},               // SFOS
        {"fernschreiber", "telegram"},          // SFOS
        {"sailorgram", "telegram"},             // SFOS (old/unmaintained)
        {"telegram", "telegram"},               // generic / Android layer
        {"yottogram", "telegram"},              // SFOS
        {"piepmatz", "twitter"},                // SFOS
        {"tweetian", "twitter"},                // SFOS
        {"twitter", "twitter"},                 // generic
        {"whatsapp", "whatsapp"},
        {"textsecure.nanuc", "signal"},         // UT: Axolotl (Signal client)
        {"cinny.danfro_cinny", "element"},      // UT: Cinny UT
        {"fluffychat.christianpauly", "element"}, // UT: FluffyChat
        {"fluffychatflutter.red", "element"},   // UT: FluffyChat (Flutter)
        {"harbour-sailtrix", "element"},        // SFOS: Sailtrix
        {"harbour-hydrogen", "element"},        // SFOS: Hydrogen
        {"harbour-matrix", "element"},          // SFOS: harbour-matrix
        {"harbour-determinant", "element"},     // SFOS: Determinant
        {"harbour-tooter", "mastodon"},         // SFOS
        {"harbour-tooterb", "mastodon"},        // SFOS (beta branch)
        {"im.kaidan.kaidan", "chat"},           // UT: Kaidan
        {"harbour-kaidan", "chat"},             // SFOS: Kaidan (via Chum)
        {"harbour-shmoose", "chat"},            // SFOS: Shmoose
        {"harbour-pingyou", "chat"},            // SFOS: PingYou
        {"harbour-sailslack", "slack"},         // SFOS (notifications unreliable)
        {"rocketchat.pparent", "chat"},         // UT (no Bangle icon → "chat")
        {"deltatouch.lotharketterer", "chat"},  // UT: DeltaTouch
        {"facebook", "facebook"},
        {"messenger", "messenger"},
        {"instagram", "instagram"},
        {"snapchat", "snapchat"},
        {"skype", "skype"},
    };

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

    bool operationRunning() override;

    //Weather
    void sendWeather(CurrentWeather *weather) override;

    void applyDeviceSetting(Amazfish::Settings s) override;
    void syncCalendar(QList<watchfish::CalendarEvent> &eventlist) override;

protected:
    virtual void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);

private:
    void parseServices();
    void initialise();
    Q_SLOT void serviceEvent(uint8_t event);
    Q_SLOT void handleRxJson(const QJsonObject &json);

    bool m_operationRunning = false;
    void setOperationRunning(bool running);
    int m_infoBatteryLevel = 0;
    int m_steps = 0;
    double m_heartrate = 0;
    QString m_firmwareVersion;
    QString m_hardwareVersion;

    QList<ActivitySample> m_samples;
    bool saveActivitySamples();
    ActivityKind::Type convertToActivityKind(const QString &bangle_kind);

    ActivitySummary m_summary;
    QString m_synced_activity_id;
    void fetchActivityRec(const QString &recId);
    QList<BangleActTrkRecord> m_activityRecords;
    bool saveSportData();
    QString activityRecordsToGpx();
    QString activityRecordsToTcx();

    QNetworkAccessManager *m_manager;
    void networkReply();

    void setTime();
    void setUserGoal(uint steps);
    void setUserGoalAlert(bool enabled);
    void setAlarms();
    int getStepsFromDb();

    QString alertIcon(const QString &appname) const;

    void forceCalendarSync(); // request list of events in bangle.js
    void syncCalendarWithDeviceIds(QList<int> &deviceIds);
    void sendCalendarEvent(int id, const watchfish::CalendarEvent &event); // send single event (create or replace)
    void removeEventReminder(int id); // remove single event from device
    QMap<QString, int> m_event_id_map; // maps calendar event uid to device id
    std::optional<QList<watchfish::CalendarEvent>> m_eventlist; // list of all events which should go to device
    int m_next_event_id = 1;

};

#endif // BANGLEJSDEVICE_H
