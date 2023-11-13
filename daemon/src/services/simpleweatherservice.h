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
        FewCloudsDay,
        ScatteredCloudsDay,
        BrokenCloudsDay,
        ShowerRainDay,
        RainDay,
        ThunderstormDay,
        SnowDay,
        MistDay,
        ClearSkyNight,
        FewCloudsNight,
        ScatteredCloudsNight,
        BrokenCloudsNight,
        ShowerRainNight,
        RainNight,
        ThunderstormNight,
        SnowNight,
        MistNight,
        Unknown // for any icon not listed above
    };

    Q_ENUM(WeatherIcons)

    WeatherIcons iconToEnum(const QString& iconName);

};

#endif // #define SIMPLEWEATHERSERVICE_H
