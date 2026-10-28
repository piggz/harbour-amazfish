#include "garminweathermessage.h"
#include "fitweatherencoder.h"

#include <QDate>
#include <QtGlobal>

void GarminWeatherMessage::parse(const QByteArray& data) {
    qDebug() << Q_FUNC_INFO << "Garmin: parsing weather request";

    if (data.size() < 10) {
        return;
    }
    WeatherRequestMessage msg;
    msg.format = quint8(data[0]);
    msg.latitude = i32le(data, 1);
    msg.longitude = i32le(data, 5);
    msg.hoursOfForecast = quint8(data[9]);
    if (mCommunicator) mCommunicator->onWeatherRequestReceived(msg);
}

namespace {

// FIT WeatherStatus values, as published in the FIT profile. Only the ones the
// OpenWeather codes below can produce are named.
const quint8 FIT_CLEAR = 0;
const quint8 FIT_PARTLY_CLOUDY = 1;
const quint8 FIT_MOSTLY_CLOUDY = 2;
const quint8 FIT_RAIN = 3;
const quint8 FIT_SNOW = 4;
const quint8 FIT_WINDY = 5;
const quint8 FIT_THUNDERSTORMS = 6;
const quint8 FIT_WINTRY_MIX = 7;
const quint8 FIT_FOG = 8;
const quint8 FIT_HAZY = 11;
const quint8 FIT_SCATTERED_SHOWERS = 13;
const quint8 FIT_SCATTERED_THUNDERSTORMS = 14;
const quint8 FIT_UNKNOWN_PRECIPITATION = 15;
const quint8 FIT_LIGHT_RAIN = 16;
const quint8 FIT_HEAVY_RAIN = 17;
const quint8 FIT_LIGHT_SNOW = 18;
const quint8 FIT_HEAVY_SNOW = 19;
const quint8 FIT_LIGHT_RAIN_SNOW = 20;
const quint8 FIT_HEAVY_RAIN_SNOW = 21;
const quint8 FIT_CLOUDY = 22;

// OpenWeather condition id to FIT WeatherStatus. CurrentWeather passes the id
// through from the API, so every id is mapped to the closest FIT value here.
quint8 fitConditionCode(int owmCode)
{
    switch (owmCode) {
    case 200: // thunderstorm with light rain
    case 201: // thunderstorm with rain
    case 202: // thunderstorm with heavy rain
    case 210: // light thunderstorm
    case 211: // thunderstorm
    case 212: // heavy thunderstorm
    case 230: // thunderstorm with light drizzle
    case 231: // thunderstorm with drizzle
    case 232: // thunderstorm with heavy drizzle
        return FIT_THUNDERSTORMS;
    case 221: // ragged thunderstorm
        return FIT_SCATTERED_THUNDERSTORMS;
    case 300: // light intensity drizzle
    case 301: // drizzle
    case 310: // light intensity drizzle rain
    case 313: // shower rain and drizzle
        return FIT_LIGHT_RAIN;
    case 302: // heavy intensity drizzle
    case 312: // heavy intensity drizzle rain
    case 314: // heavy shower rain and drizzle
        return FIT_HEAVY_RAIN;
    case 311: // drizzle rain
        return FIT_RAIN;
    case 321: // shower drizzle
        return FIT_SCATTERED_SHOWERS;
    case 500: // light rain
    case 520: // light intensity shower rain
    case 521: // shower rain
        return FIT_LIGHT_RAIN;
    case 501: // moderate rain
    case 531: // ragged shower rain
        return FIT_RAIN;
    case 502: // heavy intensity rain
    case 503: // very heavy rain
    case 504: // extreme rain
    case 522: // heavy intensity shower rain
        return FIT_HEAVY_RAIN;
    case 511: // freezing rain
        return FIT_UNKNOWN_PRECIPITATION;
    case 600: // light snow
        return FIT_LIGHT_SNOW;
    case 601: // snow
    case 620: // light shower snow
    case 621: // shower snow
        return FIT_SNOW;
    case 602: // heavy snow
    case 622: // heavy shower snow
        return FIT_HEAVY_SNOW;
    case 611: // sleet
    case 612: // light shower sleet
    case 613: // shower sleet
        return FIT_WINTRY_MIX;
    case 615: // light rain and snow
        return FIT_LIGHT_RAIN_SNOW;
    case 616: // rain and snow
        return FIT_HEAVY_RAIN_SNOW;
    case 741: // fog
        return FIT_FOG;
    case 701: // mist
    case 711: // smoke
    case 721: // haze
    case 731: // sand/dust whirls
    case 751: // sand
    case 761: // dust
    case 762: // volcanic ash
        return FIT_HAZY;
    case 771: // squalls
    case 781: // tornado
        return FIT_WINDY;
    case 800: // clear sky
        return FIT_CLEAR;
    case 801: // few clouds
    case 802: // scattered clouds
        return FIT_PARTLY_CLOUDY;
    case 803: // broken clouds
        return FIT_MOSTLY_CLOUDY;
    case 804: // overcast clouds
        return FIT_CLOUDY;
    default:
        // FIT's invalid value, so the watch shows no condition at all rather
        // than a wrong one.
        return 255;
    }
}

}

FitWeatherFrames GarminWeatherMessage::encodeWeather(const CurrentWeather* weather)
{
    FitWeatherFrames frames;
    if (!weather || !weather->city()) {
        return frames;
    }
    // CurrentWeather is zeroed before the first reply has been fetched.
    if (weather->dateTime() <= 0) {
        return frames;
    }

    GarminWeatherSnapshot snapshot;
    snapshot.timestampUnix = weather->dateTime();
    snapshot.latitude = weather->city()->latitude();
    snapshot.longitude = weather->city()->longitude();
    // CurrentWeather reports Kelvin, the encoder wants Celsius.
    snapshot.temperatureC = qRound(weather->temperature() - 273.15);
    snapshot.minTempC = qRound(weather->minTemperature() - 273.15);
    snapshot.maxTempC = qRound(weather->maxTemperature() - 273.15);
    snapshot.humidityPercent = weather->humidity();
    snapshot.windSpeedMs = weather->windSpeed();
    snapshot.windDirectionDeg = qRound(weather->windDeg());
    snapshot.conditionCode = fitConditionCode(weather->weatherCode());

    // Today also gets a daily record: the forecast list starts tomorrow, so
    // nothing else describes today. Today's precipitation probability stays
    // unknown, since OpenWeather reports none for current conditions.
    GarminWeatherDay today;
    today.dayOfWeek = QDate::currentDate().dayOfWeek() % 7; // FIT: Sunday = 0
    today.minTempC = snapshot.minTempC;
    today.maxTempC = snapshot.maxTempC;
    today.humidityPercent = snapshot.humidityPercent;
    today.windSpeedMs = snapshot.windSpeedMs;
    today.conditionCode = snapshot.conditionCode;
    snapshot.days << today;

    // Four forecast days at most. Amazfish numbers its forecast entries from
    // tomorrow, so day_of_week is today plus index plus one.
    const int forecastDays = qMin(weather->forecastCount(), 4);
    for (int i = 0; i < forecastDays; ++i) {
        const CurrentWeather::Forecast& f = weather->forecast(i);
        GarminWeatherDay day;
        day.dayOfWeek = QDate::currentDate().addDays(i + 1).dayOfWeek() % 7;
        day.minTempC = qRound(f.minTemperature() - 273.15);
        day.maxTempC = qRound(f.maxTemperature() - 273.15);
        day.humidityPercent = f.humidity();
        day.windSpeedMs = f.windMaxSpeed();
        day.conditionCode = fitConditionCode(f.weatherCode());
        // Forecast::precipProbability() is a fraction, and CurrentWeather only
        // fills it, the humidity and the condition when the API had an entry
        // around midday. Without such an entry dateTime() is still 0, so report
        // the probability as unknown instead of claiming it will not rain.
        if (f.dateTime() > 0) {
            day.precipitationProbabilityPercent = qRound(f.precipProbability() * 100.0);
        }
        snapshot.days << day;
    }

    frames.fitDefinition = FitWeatherEncoder::buildDefinitionPayload(snapshot);
    frames.fitData = FitWeatherEncoder::buildDataPayload(snapshot);
    return frames;
}
