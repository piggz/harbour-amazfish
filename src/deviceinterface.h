#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>

#include "qble/bluezadapter.h"

#include "bipdevice.h"
#include "notificationslistener.h"
#include "voicecallhandler.h"
#include "voicecallmanager.h"
#include "settingsmanager.h"
class AlertNotificationService;
class DeviceInfoService;
class HRMService;
class MiBand2Service;
class MiBandService;

class DeviceInterface : public QObject
{
    Q_OBJECT
public:
    DeviceInterface();

    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)

    Q_INVOKABLE void pair(const QString &address);
    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();

    bool ready() const;
    QString connectionState() const;

    Q_INVOKABLE DeviceInfoService *infoService() const;
    Q_INVOKABLE MiBandService *miBandService() const;
    Q_INVOKABLE MiBand2Service *miBand2Service() const;
    Q_INVOKABLE AlertNotificationService *alertNotificationService() const;
    Q_INVOKABLE HRMService *hrmService() const;

private:
    QString m_deviceAddress;
    QString m_deviceName;

    BluezAdapter m_adapter;
    BipDevice *m_bipDevice = nullptr;
    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    SettingsManager m_settings;

    void createSettings();

    void updateServiceController();

    Q_SLOT void notificationReceived(const QString &appName, const QString &summary, const QString &body);
    Q_SLOT void onActiveVoiceCallChanged();
    Q_SLOT void onActiveVoiceCallStatusChanged();

Q_SIGNALS:
    void readyChanged();
    void connectionStateChanged();
};

#endif // BIPINTERFACE_H
