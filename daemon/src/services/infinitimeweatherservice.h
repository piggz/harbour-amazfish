#ifndef INFINITIMEWEATHERSERVICE_H
#define INFINITIMEWEATHERSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

/** 00040000-78fc-48fe-8e23-433b3a1942d0 --Weather Service
 * "00040001-78fc-48fe-8e23-433b3a1942d0", --Data
 * "00040002-78fc-48fe-8e23-433b3a1942d0", --Control
 */
class InfiniTimeWeatherService : public QBLEService
{
    Q_OBJECT
public:
    static const char *UUID_SERVICE_WEATHER;
    static const char *UUID_CHARACTERISTIC_WEATHER_DATA;
    static const char *UUID_CHARACTERISTIC_WEATHER_CONTROL;

    explicit InfiniTimeWeatherService(const QString &path, QObject *parent);

    void sendWeather(CurrentWeather *weather);
};

#endif // INFINITIMEWEATHERSERVICE_H
