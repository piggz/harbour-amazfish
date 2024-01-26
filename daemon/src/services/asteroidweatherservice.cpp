#include "asteroidweatherservice.h"
#include "typeconversion.h"
#include "codec.h"

#include <QDateTime>
#include <QDebug>
#include <math.h>

const char* AsteroidWeatherService::UUID_SERVICE_WEATHER = "00008071-0000-0000-0000-00a57e401d05";
const char* AsteroidWeatherService::UUID_CHARACTERISTIC_WEAT_CITY_UUID = "00008001-0000-0000-0000-00a57e401d05";
const char* AsteroidWeatherService::UUID_CHARACTERISTIC_WEAT_IDS_UUID  = "00008002-0000-0000-0000-00a57e401d05";
const char* AsteroidWeatherService::UUID_CHARACTERISTIC_WEAT_MINT_UUID = "00008003-0000-0000-0000-00a57e401d05";
const char* AsteroidWeatherService::UUID_CHARACTERISTIC_WEAT_MAXT_UUID = "00008004-0000-0000-0000-00a57e401d05";

AsteroidWeatherService::AsteroidWeatherService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_WEATHER, path, parent)
{
    qDebug() << Q_FUNC_INFO;
}

void AsteroidWeatherService::sendWeather(CurrentWeather *weather)
{
    qDebug() << Q_FUNC_INFO;

    if (weather->forecastCount() < 5) {
        qWarning() << "WeatherService: need at least 5 days forecast, got" << weather->forecastCount();
        return;
    }
    QByteArray ids;
    QByteArray minTemps;
    QByteArray maxTemps;

    for (int f = 0; f < 5; f++) {
        CurrentWeather::Forecast fc = weather->forecast(f);
//        qDebug() << "Forecast:" << f << fc.dateTime()<< fc.weatherCode() <<  (fc.maxTemperature() - 273) <<  (fc.minTemperature() - 273) << fc.humidity() << fc.pressure() << fc.windMaxSpeed() << fc.clouds();
        ids += TypeConversion::BEfromInt16( fc.weatherCode() );
        minTemps += TypeConversion::BEfromInt16( fc.minTemperature() );
        maxTemps += TypeConversion::BEfromInt16( fc.maxTemperature() );
    }


    qDebug() << Q_FUNC_INFO << "weather ids:" << ids.toHex();
    qDebug() << Q_FUNC_INFO << "weather mint:" << minTemps.toHex();
    qDebug() << Q_FUNC_INFO << "weather maxt:" << maxTemps.toHex();


    writeValue(UUID_CHARACTERISTIC_WEAT_CITY_UUID, weather->city()->name().toLocal8Bit());
    writeValue(UUID_CHARACTERISTIC_WEAT_IDS_UUID, ids);
    writeValue(UUID_CHARACTERISTIC_WEAT_MINT_UUID, minTemps);
    writeValue(UUID_CHARACTERISTIC_WEAT_MAXT_UUID, maxTemps);

}
