
#ifndef DEVICEINTERFACE_H
#define DEVICEINTERFACE_H

#include <QObject>
#include <QtSql/QtSql>
#include <QQueue>
#include <QTimer>

#include <KDb3/KDbDriver>
#include <KDb3/KDbConnection>
#include <KDb3/KDbConnectionData>

#ifdef MER_EDITION_SAILFISH
#include <keepalive/backgroundactivity.h>
#endif

#include "abstractdevice.h"
#include "abstractfirmwareinfo.h"
#include "dbushrm.h"
#include "weather/citymanager.h"
#include "weather/currentweather.h"
#include "libwatchfish/musiccontroller.h"
#include "libwatchfish/notificationmonitor.h"
#include "libwatchfish/notification.h"
#include "libwatchfish/calendarsource.h"
#include "libwatchfish/soundprofile.h"
#include "navigationinterface.h"
#include "achievements.h"

class HRMService;
class MiBand2Service;
class MiBandService;

#define SERVICE_NAME_AMAZFISH "uk.co.piggz.amazfish"

class DeviceInterface : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", SERVICE_NAME_AMAZFISH)
public:
    DeviceInterface();
    ~DeviceInterface();

    void registerDBus();

    Q_INVOKABLE QString pair(const QString &name, const QString &deviceType, const QString &address);

    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void unpair();
    Q_INVOKABLE QString connectionState() const;
    Q_INVOKABLE int connectionStateChangedCount() const;
    Q_INVOKABLE bool operationRunning();
    Q_INVOKABLE bool supportsFeature(int f);
    Q_INVOKABLE int supportedFeatures();

    KDbConnection *dbConnection();

    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);
    Q_SIGNAL void deviceEventTriggered(const QString& event);
    Q_SIGNAL void informationChanged(int infoKey, const QString& infoValue);
    Q_SIGNAL void connectionStateChanged();

    //Functions provided by services
    Q_INVOKABLE QString prepareFirmwareDownload(const QString &path);
    Q_INVOKABLE bool startDownload();
    Q_INVOKABLE void downloadSportsData();
    Q_INVOKABLE void downloadActivityData();
    Q_INVOKABLE void refreshInformation();

    Q_INVOKABLE QString information(int i);
    Q_INVOKABLE void sendAlert(const QVariantMap &notification, bool allowDuplicate = false);
    Q_INVOKABLE void sendAlert(const Amazfish::WatchNotification &notification, bool allowDuplicate = false);
    Q_INVOKABLE void incomingCall(const QString &caller);
    Q_INVOKABLE void incomingCallEnded();
    Q_INVOKABLE void applyDeviceSetting(int s);
    Q_INVOKABLE void requestManualHeartrate();
    Q_INVOKABLE void triggerSendWeather();
    Q_INVOKABLE void updateCalendar();
    Q_INVOKABLE void reloadCities();
    Q_INVOKABLE void enableFeature(int feature);
    Q_INVOKABLE void fetchLogs();
    Q_INVOKABLE void requestScreenshot();
    Q_INVOKABLE QStringList supportedDisplayItems();
    Q_INVOKABLE void immediateAlert(int level);

private:

    int m_connectionStateChangedCount = 0;
    QString m_deviceAddress;
    QString m_deviceName;
    bool m_dbusRegistered = false;
    int m_lastBatteryLevel = 0;
    int m_lastAlertHash = 0;
    AbstractFirmwareInfo *m_firmwareInfo = nullptr;

    AbstractDevice *m_device = nullptr;

    DBusHRM *m_dbusHRM = nullptr;

    QTimer *m_refreshTimer = nullptr;
    QTimer *m_findDeviceTimer = nullptr;
    Q_SLOT void onRefreshTimer();
    void findDevice();
    int m_playedSounds = 0;

    void createSettings();
    void updateServiceController();

    void log_battery_level(int level);

    HRMService *hrmService() const;
    
    Q_SLOT void onNotification(watchfish::Notification *notification);
    Q_SLOT void onRingingChanged();
    Q_SLOT void onConnectionStateChanged();
    Q_SLOT void slot_informationChanged(AbstractDevice::Info infokey, const QString &infovalue);
    Q_SLOT void musicChanged();
    Q_SLOT void deviceEvent(AbstractDevice::Event event);
    Q_SLOT void handleButtonPressed(int presses);
    Q_SLOT void onEventTimer();
    Q_SLOT void backgroundActivityStateChanged();

    void sendBufferedNotifications();
    void scheduleNextEvent();

    //Watchfish
    watchfish::MusicController m_musicController;
#if defined(MER_EDITION_SAILFISH) || defined(UUITK_EDITION)
    watchfish::VoiceCallController m_voiceCallController;
#endif
    watchfish::NotificationMonitor m_notificationMonitor;
    watchfish::CalendarSource m_calendarSource;
    watchfish::SoundProfile m_soundProfile;

    Achievements m_achievements;

    //Notifications
    QQueue<Amazfish::WatchNotification> m_notificationBuffer;

    //Database
    KDbDriver *m_dbDriver = nullptr;
    KDbConnectionData m_connData;
    KDbConnection *m_conn = nullptr;
    void setupDatabase();
    void createTables();

    //Weather
    CityManager m_cityManager;
    CurrentWeather m_currentWeather;
    void sendWeather(CurrentWeather *weather);
    Q_SLOT void onCitiesChanged();
    Q_SLOT void onWeatherReady();

    //Navigation
    NavigationInterface m_navigationInterface;
    Q_SLOT void navigationRunningChanged(bool running);
    Q_SLOT void navigationChanged(const QString &icon, const QString &narrative, const QString &manDist, int progress);

    QDateTime m_lastWeatherSync;
    QDateTime m_lastCalendarSync;
    QDateTime m_lastActivitySync;

    //Event list for simulated event support
    QList<watchfish::CalendarEvent> m_eventlist;
#ifdef MER_EDITION_SAILFISH
    BackgroundActivity *m_backgroundActivity = nullptr;
#endif
};

#endif // BIPINTERFACE_H
