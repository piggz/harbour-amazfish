#ifndef IMMEDIATEALERTSERVERSERVICE_H
#define IMMEDIATEALERTSERVERSERVICE_H

#include "qble/qblelocalservice.h"

// https://www.bluetooth.com/specifications/specs/immediate-alert-service-1-0/
//
// This class serves the IAS on the desktop/phone side so that an InfiniTime watch
// acting as GATT client (FindMyPhone screen) can write to the Alert Level
// characteristic and trigger an alert on the companion device.
//
// Contrast with ImmediateAlertService (client side) which writes *to* the watch.

class ImmediateAlertServerService : public QBLELocalService
{
    Q_OBJECT
public:
    explicit ImmediateAlertServerService(QDBusConnection bus, unsigned int serviceIndex, QObject *parent = nullptr);

    static const char *UUID_SERVICE_IMMEDIATE_ALERT;
    static const char *UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL;

    enum class Levels : uint8_t { NoAlert = 0, MildAlert = 1, HighAlert = 2 };
    Q_ENUM(Levels)

Q_SIGNALS:
    // Emitted when the watch writes a new alert level
    void alertLevelChanged(int level);

private Q_SLOTS:
    void onValueWritten(const QByteArray &value);
};

#endif // IMMEDIATEALERTSERVERSERVICE_H
