#ifndef ASTEROID_TIME_SERVICE__H
#define ASTEROID_TIME_SERVICE__H

#include <QObject>

#include "qble/qbleservice.h"
#include "abstractdevice.h"

class AsteroidTimeService : public QBLEService
{
    Q_OBJECT
public:
    AsteroidTimeService(const QString &path, QObject *parent);
    static const char *UUID_SERVICE_ASTEROID_TIME;
    static const char *UUID_CHARACTERISTIC_ASTEROID_TIME_SET;

    void setCurrentTime();
};

#endif // ASTEROID_TIME_SERVICE__H
