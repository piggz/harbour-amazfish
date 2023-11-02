#include "simpleweatherservice.h"
#include "typeconversion.h"
#include "codec.h"

#include <QDateTime>
#include <QDebug>
#include <math.h>

const char* SimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER = "00090000-78fc-48fe-8e23-433b3a1942d0";
const char* SimpleWeatherService::UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA = "00090001-78fc-48fe-8e23-433b3a1942d0";

SimpleWeatherService::SimpleWeatherService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_SIMPLE_WEATHER, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}


void SimpleWeatherService::sendWeather(CurrentWeather *weather)
{

    qDebug() << "WEATHER"
        << weather->temperature()
        << weather->minTemperature()
        << weather->maxTemperature()
        << (int)iconToEnum(weather->weatherIcon())
        << weather->windDeg()
        << weather->windSpeed()
        << weather->windGusts()
        << weather->humidity()
        << weather->clouds()
        << weather->dateTime()
    ;

//    writeValue(UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA, l.toCBOR());

}

SimpleWeatherService::WeatherIcons SimpleWeatherService::iconToEnum(const QString& iconName) {
    if (iconName == "01d.png") return WeatherIcons::ClearSkyDay;
    if (iconName == "01n.png") return WeatherIcons::ClearSkyNight;
    if (iconName == "02d.png") return WeatherIcons::FewCloudsDay;
    if (iconName == "02n.png") return WeatherIcons::FewCloudsNight;
    if (iconName == "03d.png") return WeatherIcons::ScatteredCloudsDay;
    if (iconName == "03n.png") return WeatherIcons::ScatteredCloudsNight;
    if (iconName == "04d.png") return WeatherIcons::BrokenCloudsDay;
    if (iconName == "04n.png") return WeatherIcons::BrokenCloudsNight;
    if (iconName == "09d.png") return WeatherIcons::ShowerRainDay;
    if (iconName == "09n.png") return WeatherIcons::ShowerRainNight;
    if (iconName == "10d.png") return WeatherIcons::RainDay;
    if (iconName == "10n.png") return WeatherIcons::RainNight;
    if (iconName == "11d.png") return WeatherIcons::ThunderstormDay;
    if (iconName == "11n.png") return WeatherIcons::ThunderstormNight;
    if (iconName == "13d.png") return WeatherIcons::SnowDay;
    if (iconName == "13n.png") return WeatherIcons::SnowNight;
    if (iconName == "50d.png") return WeatherIcons::MistDay;
    if (iconName == "50n.png") return WeatherIcons::MistNight;
    return WeatherIcons::Unknown;
}
