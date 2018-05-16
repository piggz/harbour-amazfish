#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>

#include "bipdevice.h"
#include "notificationslistener.h"
#include "voicecallhandler.h"
#include "voicecallmanager.h"
#include "settingsmanager.h"
class AlertNotificationService;
class BipInfoService;
class HRMService;
class MiBand2Service;
class MiBandService;

class BipInterface : public QObject
{
    Q_OBJECT
public:
    BipInterface();

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)

    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();

    bool ready() const;
    QString connectionState() const;

    Q_INVOKABLE BipInfoService *infoService() const;
    Q_INVOKABLE MiBandService *miBandService() const;
    Q_INVOKABLE MiBand2Service *miBand2Service() const;
    Q_INVOKABLE AlertNotificationService *alertNotificationService() const;
    Q_INVOKABLE HRMService *hrmService() const;

    void enableNotifications();
private:
    bool m_ready = false;
    int m_reconnections = 0;
    bool m_autoReconnect = true;
    bool m_triedReconnect = false;
    bool m_hasInitialised = false;

    QString m_connectionState;
    QString m_address;

    BipDevice *m_bipDevice = nullptr;
    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    SettingsManager m_settings;

    void createSettings();

    void updateServiceController();

    Q_SLOT void serviceReady(bool r);
    Q_SLOT void authenticated();
    Q_SLOT void notificationReceived(const QString &appName, const QString &summary, const QString &body);
    Q_SLOT void onActiveVoiceCallChanged();
    Q_SLOT void onActiveVoiceCallStatusChanged();

Q_SIGNALS:
    void readyChanged();
    void connectionStateChanged();
};

#endif // BIPINTERFACE_H
