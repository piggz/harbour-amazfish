#ifndef BIPINTERFACE_H
#define BIPINTERFACE_H

#include <QObject>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>

#include "bipinfoservice.h"

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

    bool ready() const;
    QString connectionState() const;

    Q_INVOKABLE BipInfoService *infoService() const;

private:
    bool m_ready = false;
    QString m_connectionState;
    QString m_address;
    QLowEnergyController *m_controller = nullptr;

    void updateServiceController();

    BipInfoService *m_infoService = nullptr;
    QList<BipService *>m_genericServices;

Q_SIGNALS:
    void readyChanged();
    void connectionStateChanged();
};

#endif // BIPINTERFACE_H
