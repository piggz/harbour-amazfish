#include "fitweatherencoder.h"
#include "garmintypes.h"

#include <QtGlobal>

namespace {

// FIT base type ids, from the FIT protocol base type table.
const quint8 FIT_BASE_ENUM = 0x00;
const quint8 FIT_BASE_SINT8 = 0x01;
const quint8 FIT_BASE_UINT8 = 0x02;
const quint8 FIT_BASE_UINT16 = 0x84;
const quint8 FIT_BASE_SINT32 = 0x85;
const quint8 FIT_BASE_UINT32 = 0x86;

// FIT global message number for WEATHER_CONDITIONS, from the FIT profile.
const quint16 FIT_MESSAGE_WEATHER_CONDITIONS = 128;

// FIT WeatherReport ordinals.
const quint8 REPORT_CURRENT = 0;
const quint8 REPORT_DAILY_FORECAST = 2;

// Local message types. FIT only registers a local type once a record is added
// under it, so leaving out the hourly_forecast report leaves daily at type 1.
const quint8 LOCAL_TYPE_CURRENT = 0;
const quint8 LOCAL_TYPE_DAILY = 1;

struct WeatherField {
    quint8 number;
    quint8 size;
    quint8 baseType;
};

// Field order only has to match between a definition and the records that
// follow it, because FIT data records are positional, not tagged. It does not
// have to match the field order other Garmin apps send.
//
// FIXME field 18 (atmospheric_pressure): Parse the value from the API reply
// in lib/src/weather before adding the field.
//
// FIXME field 3 (wind_direction) is left out of the daily report because
// CurrentWeather::Forecast never sets windDirection, so it is always 0.
const WeatherField CURRENT_FIELDS[] = {
    { 0, 1, FIT_BASE_ENUM }, // weather_report
    { 1, 1, FIT_BASE_SINT8 }, // temperature
    { 2, 1, FIT_BASE_ENUM }, // condition
    { 3, 2, FIT_BASE_UINT16 }, // wind_direction
    { 4, 2, FIT_BASE_UINT16 }, // wind_speed
    { 7, 1, FIT_BASE_UINT8 }, // relative_humidity
    { 9, 4, FIT_BASE_UINT32 }, // observed_at_time
    { 10, 4, FIT_BASE_SINT32 }, // observed_location_lat
    { 11, 4, FIT_BASE_SINT32 }, // observed_location_long
    { 13, 1, FIT_BASE_SINT8 }, // high_temperature
    { 14, 1, FIT_BASE_SINT8 }, // low_temperature
    { 253, 4, FIT_BASE_UINT32 }, // timestamp
};

const WeatherField DAILY_FIELDS[] = {
    { 0, 1, FIT_BASE_ENUM }, // weather_report
    { 253, 4, FIT_BASE_UINT32 }, // timestamp
    { 14, 1, FIT_BASE_SINT8 }, // low_temperature
    { 13, 1, FIT_BASE_SINT8 }, // high_temperature
    { 2, 1, FIT_BASE_ENUM }, // condition
    { 5, 1, FIT_BASE_UINT8 }, // precipitation_probability
    { 12, 1, FIT_BASE_ENUM }, // day_of_week
    { 7, 1, FIT_BASE_UINT8 }, // relative_humidity
    { 4, 2, FIT_BASE_UINT16 }, // wind_speed
};

// GARMIN_TIME_OFFSET is the shift from the Unix epoch to the FIT epoch of 1989-12-31.
static inline quint32 toGarminEpoch(qint64 unixSeconds)
{
    return quint32(qBound(qint64(0), unixSeconds - GARMIN_TIME_OFFSET, qint64(0xFFFFFFFE)));
}

static inline qint32 degreesToSemicircles(double degrees)
{
    return qint32(qBound(-2147483648.0, degrees * (2147483648.0 / 180.0), 2147483647.0));
}

// Normalize any int to [0,360) avoiding overflows
static inline int normalizeDegrees(int degrees)
{
    return ((degrees % 360) + 360) % 360;
}

// FIT wind_speed is in millimetres per second.
static inline quint16 windSpeedMillimetresPerSecond(double metresPerSecond)
{
    return quint16(quint16(qBound(0, qRound(metresPerSecond * 1000.0), 0xFFFE)));
}

static inline void appendRecordDefinition(QByteArray& payload, quint8 localType, const WeatherField* fields, int fieldCount)
{
    payload.append(char(0x40 | localType)); // definition header for this local type
    payload.append(char(0)); // reserved
    payload.append(char(0)); // byte order: 0 means little endian
    writeU16le(payload, FIT_MESSAGE_WEATHER_CONDITIONS);
    payload.append(char(fieldCount));
    for (int i = 0; i < fieldCount; ++i) {
        payload.append(char(fields[i].number));
        payload.append(char(fields[i].size));
        payload.append(char(fields[i].baseType));
    }
}

static inline void appendCurrentRecord(QByteArray& payload, const GarminWeatherSnapshot& snapshot)
{
    const quint32 timestamp = toGarminEpoch(snapshot.timestampUnix);

    payload.append(char(LOCAL_TYPE_CURRENT));
    payload.append(char(REPORT_CURRENT));
    writeTemperatureC(payload, snapshot.temperatureC);
    payload.append(char(snapshot.conditionCode));
    writeU16le(payload, quint16(normalizeDegrees(snapshot.windDirectionDeg)));
    writeU16le(payload, windSpeedMillimetresPerSecond(snapshot.windSpeedMs));
    writePercent(payload, snapshot.humidityPercent);
    writeU32le(payload, timestamp); // observed_at_time
    writeU32le(payload, quint32(degreesToSemicircles(snapshot.latitude)));
    writeU32le(payload, quint32(degreesToSemicircles(snapshot.longitude)));
    writeTemperatureC(payload, snapshot.maxTempC);
    writeTemperatureC(payload, snapshot.minTempC);
    writeU32le(payload, timestamp); // timestamp
}

// Every daily record carries the time the data was taken, not the time of that
// day. day_of_week is what tells the days apart.
static inline void appendDailyRecord(QByteArray& payload, const GarminWeatherDay& day, qint64 takenAtUnix)
{
    payload.append(char(LOCAL_TYPE_DAILY));
    payload.append(char(REPORT_DAILY_FORECAST));
    writeU32le(payload, toGarminEpoch(takenAtUnix));
    writeTemperatureC(payload, day.minTempC);
    writeTemperatureC(payload, day.maxTempC);
    payload.append(char(day.conditionCode));
    writePercent(payload, day.precipitationProbabilityPercent);
    payload.append(char(quint8(day.dayOfWeek)));
    writePercent(payload, day.humidityPercent);
    writeU16le(payload, windSpeedMillimetresPerSecond(day.windSpeedMs));
}

}

QByteArray FitWeatherEncoder::buildDefinitionPayload(const GarminWeatherSnapshot& snapshot)
{
    QByteArray payload;
    appendRecordDefinition(payload, LOCAL_TYPE_CURRENT, CURRENT_FIELDS, int(sizeof(CURRENT_FIELDS) / sizeof(CURRENT_FIELDS[0])));
    // Only define the daily report when there is a day to send, so that no
    // local type is ever defined without a record under it.
    if (!snapshot.days.isEmpty()) {
        appendRecordDefinition(payload, LOCAL_TYPE_DAILY, DAILY_FIELDS, int(sizeof(DAILY_FIELDS) / sizeof(DAILY_FIELDS[0])));
    }
    return payload;
}

QByteArray FitWeatherEncoder::buildDataPayload(const GarminWeatherSnapshot& snapshot)
{
    QByteArray payload;
    appendCurrentRecord(payload, snapshot);
    for (int i = 0; i < snapshot.days.size(); ++i) {
        appendDailyRecord(payload, snapshot.days.at(i), snapshot.timestampUnix);
    }
    return payload;
}
