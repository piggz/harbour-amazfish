#include "immediatealertserverservice.h"
#include "qble/qblelocalcharacteristic.h"

#include <QDebug>

const char *ImmediateAlertServerService::UUID_SERVICE_IMMEDIATE_ALERT = "00001802-0000-1000-8000-00805f9b34fb";
const char *ImmediateAlertServerService::UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL = "00002a06-0000-1000-8000-00805f9b34fb";

// ---------------------------------------------------------------------------
// Alert Level characteristic — write-without-response, no read
// InfiniTime uses ble_gattc_write_no_rsp() so the flag must be
// "write-without-response" (not "write").
// ---------------------------------------------------------------------------
class AlertLevelCharacteristic : public QBLELocalCharacteristic
{
public:
    AlertLevelCharacteristic(QDBusConnection bus, unsigned int index, QBLELocalService *service)
        : QBLELocalCharacteristic(bus, index, QString::fromLatin1(ImmediateAlertServerService::UUID_CHARACTERISTIC_IMMEDIATE_ALERT_LEVEL),
            { QStringLiteral("write-without-response") },
            service, service)
    {
    }
    // WriteValue is inherited from QBLELocalCharacteristic which already
    // emits valueWritten(value) — that is all we need.
};

// ---------------------------------------------------------------------------

ImmediateAlertServerService::ImmediateAlertServerService(QDBusConnection bus, unsigned int serviceIndex, QObject *parent)
    : QBLELocalService(bus, serviceIndex, QString::fromLatin1(UUID_SERVICE_IMMEDIATE_ALERT), QStringLiteral("/uk/co/piggz/amazfish/gatt"), parent)
{
    auto *alertLevel = new AlertLevelCharacteristic(bus, 0, this);
    addCharacteristic(alertLevel);

    connect(alertLevel, &QBLELocalCharacteristic::valueWritten, this, &ImmediateAlertServerService::onValueWritten);
}

void ImmediateAlertServerService::onValueWritten(const QByteArray &value)
{
    if (value.isEmpty()) {
        qWarning() << Q_FUNC_INFO << "Received empty write on Alert Level";
        return;
    }

    int level = static_cast<uint8_t>(value.at(0));
    qDebug() << Q_FUNC_INFO << "Alert level received from watch:" << level;
    emit alertLevelChanged(level);
}
