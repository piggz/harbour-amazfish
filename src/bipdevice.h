#ifndef BIPDEVICE_H
#define BIPDEVICE_H

#include "qble/qbledevice.h"
#include "qble/qbleservice.h"

#include "deviceinfoservice.h"
#include "mibandservice.h"
#include "miband2service.h"
#include "alertnotificationservice.h"
#include "hrmservice.h"

#include <QTimer>

class BipDevice : public QBLEDevice
{
    Q_OBJECT
public:
    BipDevice();
    static const char* UUID_SERVICE_ALERT_NOTIFICATION;
    static const char* UUID_SERVICE_MIBAND2;
    static const char* UUID_SERVICE_MIBAND;
    static const char* UUID_SERVICE_HRM;
    static const char* UUID_SERVICE_DEVICEINFO;

    virtual QString pair();
    virtual void pairAsync();

    virtual void connectToDevice();
    virtual void disconnectFromDevice();

    QString connectionState() const;
    Q_SIGNAL void connectionStateChanged();
    Q_SIGNAL void message(const QString &text);

    Q_SLOT void authenticated(bool ready);
private:
    void parseServices();
    bool m_needsAuth = false;
    bool m_pairing = false;
    bool m_ready = false;
    bool m_hasInitialised = false;
    bool m_autoreconnect = true;
    QString m_connectionState;
    QTimer *m_reconnectTimer;

    void initialise();
    void setConnectionState(const QString &state);
    void reconnectionTimer();

    Q_SLOT void onPropertiesChanged(QString interface, QVariantMap map, QStringList list);
};

#endif // BIPDEVICE_H
