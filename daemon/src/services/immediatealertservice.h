#ifndef IMMEDIATE_ALERT_SERVICE_H
#define IMMEDIATE_ALERT_SERVICE_H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

// https://www.bluetooth.com/specifications/specs/immediate-alert-service-1-0/

class ImmediateAlertService : public QBLEService
{
    Q_OBJECT
public:
    ImmediateAlertService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_IMMEDIATE_ALERT;
    static const char* UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL;
    enum class Levels : uint8_t { NoAlert = 0, MildAlert = 1, HighAlert = 2 };

    Q_INVOKABLE void refreshInformation();

    int alertLevel() const;

    Q_SIGNAL void informationChanged(AbstractDevice::Info key, const QString &val);

private:
    int m_alertLevel;
    Q_SLOT void characteristicRead(const QString &c, const QByteArray &value);
};

#endif // IMMEDIATE_ALERT_SERVICE_H
