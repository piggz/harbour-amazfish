#ifndef SIMPLEWEATHERSERVICE_H
#define SIMPLEWEATHERSERVICE_H

#include <QObject>
#include "qble/qbleservice.h"
#include "devices/abstractdevice.h"

class SimpleWeatherService : public QBLEService
{
    Q_OBJECT
public:
    static const char *UUID_SERVICE_SIMPLE_WEATHER;
    static const char *UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA;

    explicit SimpleWeatherService(const QString &path, QObject *parent);


    void sendWeather(CurrentWeather *weather);

    enum class WeatherIcons {
        ClearSkyDay,
        ClearSkyNight,
        FewCloudsDay,
        FewCloudsNight,
        ScatteredCloudsDay,
        ScatteredCloudsNight,
        BrokenCloudsDay,
        BrokenCloudsNight,
        ShowerRainDay,
        ShowerRainNight,
        RainDay,
        RainNight,
        ThunderstormDay,
        ThunderstormNight,
        SnowDay,
        SnowNight,
        MistDay,
        MistNight,
        Unknown // for any icon not listed above
    };

    WeatherIcons iconToEnum(const QString& iconName);

};

#endif // #define SIMPLEWEATHERSERVICE_H
