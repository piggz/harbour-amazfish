#ifndef CURRENTTIMESERVICE_H
#define CURRENTTIMESERVICE_H

#include <QObject>

#include "qble/qbleservice.h"
#include "abstractdevice.h"

/*
{00001805-0000-1000-8000-00805f9b34fb} Pinetime service
--00002a2b-0000-1000-8000-00805f9b34fb //Time
*/
class CurrentTimeService : public QBLEService
{
    Q_OBJECT
public:
    CurrentTimeService(const QString &path, QObject *parent);
    static const char *UUID_SERVICE_CURRENT_TIME;
    static const char *UUID_CHARACTERISTIC_CURRENT_TIME;

    void setCurrentTime();
    QDateTime currentTime();
};

#endif // CURRENTTIMESERVICE_H
