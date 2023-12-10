#include "simpleweatherservice.h"
#include "typeconversion.h"
#include "codec.h"

#include <QDateTime>
#include <QDebug>
#include <math.h>

const char* SimpleWeatherService::UUID_SERVICE_SIMPLE_WEATHER = "00050000-78fc-48fe-8e23-433b3a1942d0";
const char* SimpleWeatherService::UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA = "00050001-78fc-48fe-8e23-433b3a1942d0";

SimpleWeatherService::SimpleWeatherService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_SIMPLE_WEATHER, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}


void SimpleWeatherService::sendWeather(CurrentWeather *weather)
{

    qDebug() << "Current weather data"
        << QDateTime::fromSecsSinceEpoch(weather->dateTime())
        << weather->dateTime()
        << weather->temperature() - 273.15
        << weather->minTemperature() - 273.15
        << weather->maxTemperature() - 273.15
        << weather->city()->name()
        << weather->weatherIcon() 
        << QMetaEnum::fromType<SimpleWeatherService::WeatherIcons>().valueToKey(
                (int)iconToEnum(weather->weatherIcon())
           )
//        << weather->clouds()
//        << weather->humidity()
//        << weather->windDeg()
//        << weather->windSpeed()
//        << weather->windGusts()
    ;

    QByteArray cityNameBytes = weather->city()->name().toLocal8Bit().left(32);
    if(cityNameBytes.size() < 32) {
        cityNameBytes.append(32-cityNameBytes.size(), 0x00);
    }

    QByteArray weatherBytes;

    weatherBytes += TypeConversion::fromInt8(0); // message type
    weatherBytes += TypeConversion::fromInt8(0); // version information
    weatherBytes += TypeConversion::fromInt64(weather->dateTime());
    weatherBytes += TypeConversion::fromInt8( weather->temperature() - 273.15 );
    weatherBytes += TypeConversion::fromInt8( weather->minTemperature() - 273.15 );
    weatherBytes += TypeConversion::fromInt8( weather->maxTemperature() - 273.15 );
    weatherBytes += cityNameBytes;
    weatherBytes += TypeConversion::fromInt8( (int)iconToEnum(weather->weatherIcon()) );

//    weatherBytes += TypeConversion::fromInt8( weather->clouds() );
//    weatherBytes += TypeConversion::fromInt8( weather->humidity() );

//    weatherBytes += TypeConversion::fromInt16( (int)(100 * weather->windDeg()));
//    weatherBytes += TypeConversion::fromInt16((int)(100 * weather->windSpeed()));
//    weatherBytes += TypeConversion::fromInt16((int)(100 * weather->windGusts()));

    qDebug() << "Weather bytes" << weatherBytes.toHex();

    writeValue(UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA, weatherBytes);


    int fcDays = std::min(weather->forecastCount(), 5);

    QByteArray forecastBytes;
    forecastBytes += TypeConversion::fromInt8(1); // message type
    forecastBytes += TypeConversion::fromInt8(0); // version information
    forecastBytes += TypeConversion::fromInt64(weather->dateTime());
    forecastBytes += TypeConversion::fromInt8(fcDays);



    for (int f = 0; (f < fcDays); f++) {
        CurrentWeather::Forecast fc = weather->forecast(f);
//        qDebug() << "Forecast:" << f << fc.dateTime()<< fc.weatherCode() <<  (fc.maxTemperature() - 273) <<  (fc.minTemperature() - 273) << fc.humidity() << fc.pressure() << fc.windMaxSpeed() << fc.clouds();


    qDebug() << "Forecast Day" << f
        << QDateTime::fromSecsSinceEpoch(fc.dateTime())
        << fc.dateTime()
        << fc.minTemperature() - 273.15
        << fc.maxTemperature() - 273.15
        << fc.weatherIcon()
        << QMetaEnum::fromType<SimpleWeatherService::WeatherIcons>().valueToKey(
                (int)iconToEnum(fc.weatherIcon())
           )
    ;

        forecastBytes += TypeConversion::fromInt8( fc.minTemperature() - 273.15 );
        forecastBytes += TypeConversion::fromInt8( fc.maxTemperature() - 273.15 );
        forecastBytes += TypeConversion::fromInt8( (int)iconToEnum(fc.weatherIcon()) );

    }

    qDebug() << "Forecast bytes" << forecastBytes.toHex();

    writeValue(UUID_CHARACTERISTIC_SIMPLE_WEATHER_DATA, forecastBytes);

}

SimpleWeatherService::WeatherIcons SimpleWeatherService::iconToEnum(const QString& iconName) {
    if (iconName == "01d") return WeatherIcons::ClearSky;
    if (iconName == "01n") return WeatherIcons::ClearSky;
    if (iconName == "02d") return WeatherIcons::FewClouds;
    if (iconName == "02n") return WeatherIcons::FewClouds;
    if (iconName == "03d") return WeatherIcons::ScatteredClouds;
    if (iconName == "03n") return WeatherIcons::ScatteredClouds;
    if (iconName == "04d") return WeatherIcons::BrokenClouds;
    if (iconName == "04n") return WeatherIcons::BrokenClouds;
    if (iconName == "09d") return WeatherIcons::ShowerRain;
    if (iconName == "09n") return WeatherIcons::ShowerRain;
    if (iconName == "10d") return WeatherIcons::Rain;
    if (iconName == "10n") return WeatherIcons::Rain;
    if (iconName == "11d") return WeatherIcons::Thunderstorm;
    if (iconName == "11n") return WeatherIcons::Thunderstorm;
    if (iconName == "13d") return WeatherIcons::Snow;
    if (iconName == "13n") return WeatherIcons::Snow;
    if (iconName == "50d") return WeatherIcons::Mist;
    if (iconName == "50n") return WeatherIcons::Mist;
    return WeatherIcons::Unknown;
}
