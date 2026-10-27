#ifndef FITWEATHERENCODER_H
#define FITWEATHERENCODER_H

#include <QByteArray>
#include <QVector>

// One day of the daily_forecast report of a Garmin FIT weather record.
// Filled by GarminWeatherMessage from a CurrentWeather forecast entry.
struct GarminWeatherDay {
    // FIT DayOfWeek ordinal: Sunday = 0 .. Saturday = 6.
    int dayOfWeek = 0;
    int minTempC = 0;
    int maxTempC = 0;
    int humidityPercent = 0;
    double windSpeedMs = 0.0;
    // -1 means unknown and is written as FIT's own invalid value, so the watch
    // shows "no data" instead of "will not rain".
    int precipitationProbabilityPercent = -1;
    // FIT WeatherStatus value, see the FIT profile.
    quint8 conditionCode = 0;
};

// Current conditions plus the days to send. Everything is already in the units
// the encoder needs: Celsius, m/s, percent, FIT WeatherStatus ordinal.
// GarminWeatherMessage converts from the units CurrentWeather reports.
struct GarminWeatherSnapshot {
    qint64 timestampUnix = 0;
    double latitude = 0.0;
    double longitude = 0.0;

    int temperatureC = 0;
    int minTempC = 0;
    int maxTempC = 0;
    int humidityPercent = 0;
    double windSpeedMs = 0.0;
    int windDirectionDeg = 0;
    quint8 conditionCode = 0;

    QVector<GarminWeatherDay> days;
};

// Encodes a snapshot as the payload pair of a GFDI FitDefinition (5011) and a
// GFDI FitData (5012) message, using FIT global message 128. The payloads are
// bare: CommunicatorV2::sendMessage() adds the GFDI framing, the CRC and the
// COBS encoding itself. Only current conditions and daily forecasts are sent,
// since those are the reports CurrentWeather can supply.
//
// Both builders must be called with the same snapshot: which records a data
// message holds depends on the snapshot, and the definition must describe them.
namespace FitWeatherEncoder {
QByteArray buildDefinitionPayload(const GarminWeatherSnapshot& snapshot);
QByteArray buildDataPayload(const GarminWeatherSnapshot& snapshot);
}

#endif // FITWEATHERENCODER_H
