#include "pebbleservice.h"

const char* PebbleService::UUID_SERVICE_PEBBLE                       = "0000fed9-0000-1000-8000-00805f9b34fb";
const char* PebbleService::UUID_CHARACTERISTIC_CONNECTIVITY          = "00000001-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_PAIRING_TRIGGER       = "00000002-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_MTU                   = "00000003-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_CHARACTERISTIC_CONNECTION_PARAMETERS = "00000005-328e-0fbb-c642-1aa6699bdada";
const char* PebbleService::UUID_DESCRIPTOR_CONFIGURATION             = "00002902-0000-1000-8000-00805f9b34fb";

PebbleService::PebbleService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_PEBBLE , path, parent)
{
    qDebug() << Q_FUNC_INFO;
}
