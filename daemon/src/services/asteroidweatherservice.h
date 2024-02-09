#ifndef ASTEROIDWEATHERSERVICE_H
#define ASTEROIDWEATHERSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class AsteroidWeatherService : public QBLEService
{
    Q_OBJECT
public:

    static const char* UUID_SERVICE_WEATHER;
    static const char* UUID_CHARACTERISTIC_WEAT_CITY_UUID;
    static const char* UUID_CHARACTERISTIC_WEAT_IDS_UUID;
    static const char* UUID_CHARACTERISTIC_WEAT_MINT_UUID;
    static const char* UUID_CHARACTERISTIC_WEAT_MAXT_UUID;

    explicit AsteroidWeatherService(const QString &path, QObject *parent);

    void sendWeather(CurrentWeather *weather);

};

#endif // #define ASTEROIDWEATHERSERVICE_H
