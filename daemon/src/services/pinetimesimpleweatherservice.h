#ifndef PINETIMESIMPLEWEATHERSERVICE_H
#define PINETIMESIMPLEWEATHERSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class PineTimeSimpleWeatherService : public QBLEService
{
    Q_OBJECT
public:
    static const char *UUID_SERVICE_SIMPLE_WEATHER;
    static const char *UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA;

    explicit PineTimeSimpleWeatherService(const QString &path, QObject *parent);


    void sendWeather(CurrentWeather *weather);

    enum class WeatherIcons {
        ClearSky = 0,
        FewClouds = 1,
        ScatteredClouds = 2,
        BrokenClouds = 3,
        ShowerRain = 4,
        Rain = 5,
        Thunderstorm = 6,
        Snow = 7,
        Mist = 8,
        Unknown = 255 // for any icon not listed above
    };

    Q_ENUM(WeatherIcons)

    WeatherIcons iconToEnum(const QString& iconName);

};

#endif // #define PINETIMESIMPLEWEATHERSERVICE_H
