#include "pebbleservice.h"
#include "qble/qblecharacteristic.h"

const char* PebbleService::UUID_SERVICE_PEBBLE                       = "0000fed9-0000-1000-8000-00805f9b34fb";
const char* PebbleService::UUID_CHARACTERISTIC_CONNECTIVITY          = "00000001-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_PAIRING_TRIGGER       = "00000002-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_MTU                   = "00000003-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_CONNECTION_PARAMETERS = "00000005-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_DESCRIPTOR_CONFIGURATION             = "00002902-0000-1000-8000-00805f9b34fb";

PebbleService::PebbleService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_PEBBLE , path, parent)
{
    qDebug() << Q_FUNC_INFO;

    connect(this, &QBLEService::characteristicChanged, this, &PebbleService::onCharacteristicChanged);
    connect(this, &QBLEService::characteristicRead, this, &PebbleService::onCharacteristicChanged);
    enableNotification(UUID_CHARACTERISTIC_CONNECTIVITY);

    // Firmware 3 has a read-only trigger: reading it starts the pairing instead
    connect(this, &QBLEService::characteristicWriteFailed, this, [this](const QString &characteristic, const QString &error) {
        if (characteristic == UUID_CHARACTERISTIC_PAIRING_TRIGGER) {
            qDebug() << "Pairing trigger write failed, reading it instead:" << error;
            readAsync(UUID_CHARACTERISTIC_PAIRING_TRIGGER);
        }
    });
}

void PebbleService::triggerPairing()
{
    // 0x04: the watch sends a security request, the phone's pairing agent answers it (as the official iOS app)
    QBLECharacteristic *trigger = characteristic(UUID_CHARACTERISTIC_PAIRING_TRIGGER);
    if (trigger) {
        trigger->writeAsyncChecked(QByteArray(1, 0x04));
    }
}

void PebbleService::onCharacteristicChanged(const QString &characteristic, const QByteArray &value)
{
    // Connectivity flags: 1 connected, 2 paired, 4 encrypted, 8 has bonded gateway
    qDebug() << Q_FUNC_INFO << characteristic << value.toHex();
}
