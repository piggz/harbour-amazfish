#ifndef PEBBLE_SERVICE__H
#define PEBBLE_SERVICE__H

#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"


class PebbleService : public QBLEService
{
    Q_OBJECT
public:
    PebbleService(const QString &path, QObject *parent);

    static const char* UUID_SERVICE_PEBBLE;
    static const char* UUID_CHARACTERISTIC_CONNECTIVITY;
    static const char* UUID_CHARACTERISTIC_PAIRING_TRIGGER;
    static const char* UUID_CHARACTERISTIC_MTU;
    static const char* UUID_CHARACTERISTIC_CONNECTION_PARAMETERS;
    static const char* UUID_DESCRIPTOR_CONFIGURATION;

};

#endif // PEBBLE_SERVICE__H
