#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

#include "bipinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "notificationslistener.h"
#include "voicecallhandler.h"
#include "voicecallmanager.h"

class BipInterface : public QObject
{
    Q_OBJECT
public:
    BipInterface();

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)

    const QString UUID_SERVICE_ALERT_NOTIFICATION = "{00001811-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_NEW_ALERT = "{00002a46-0000-1000-8000-00805f9b34fb}";
    const QString UUID_CHARACTERISTIC_ALERT_CONTROL = "{00002a44-0000-1000-8000-00805f9b34fb}";

    Q_INVOKABLE void connectToDevice(const QString &address);
    Q_INVOKABLE void disconnect();

    bool ready() const;
    QString connectionState() const;

    Q_INVOKABLE BipInfoService *infoService() const;
    Q_INVOKABLE MiBandService *miBandService() const;
    Q_INVOKABLE MiBand2Service *miBand2Service() const;
    Q_INVOKABLE AlertNotificationService *alertNotificationService() const;

    void enableNotifications();
private:
    bool m_ready = false;
    QString m_connectionState;
    QString m_address;
    QLowEnergyController *m_controller = nullptr;
    BipInfoService *m_infoService = nullptr;
    MiBandService *m_mibandService = nullptr;
    MiBand2Service *m_miband2Service = nullptr;
    AlertNotificationService *m_alertNotificationService = nullptr;

    NotificationsListener *m_notificationListener = nullptr;
    VoiceCallManager *m_voiceCallManager = nullptr;

    QList<BipService *>m_genericServices;

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
