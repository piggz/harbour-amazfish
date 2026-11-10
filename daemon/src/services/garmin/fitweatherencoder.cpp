#include "fitweatherencoder.h"
#include "garmintypes.h"

#include <QtGlobal>

namespace {

// FIT base type ids, from the FIT protocol base type table.
const quint8 FIT_BASE_ENUM = 0x00;
const quint8 FIT_BASE_SINT8 = 0x01;
const quint8 FIT_BASE_UINT8 = 0x02;
const quint8 FIT_BASE_STRING = 0x07;
const quint8 FIT_BASE_UINT16 = 0x84;
const quint8 FIT_BASE_SINT32 = 0x85;
const quint8 FIT_BASE_UINT32 = 0x86;
const quint8 FIT_BASE_FLOAT32 = 0x88;

// FIT invalid values, the same fill bytes Gadgetbridge writes for unset fields.
// ENUM and UINT8 both fill with 0xFF, SINT8 with 0x7F, STRING with 0x00 and
// FLOAT32 with the bit pattern 0xFFFFFFFF (a quiet NaN).
const quint8 FIT_INVALID_ENUM = 0xFF;
const quint8 FIT_INVALID_SINT8 = 0x7F;
const quint32 FIT_INVALID_UINT32 = 0xFFFFFFFF;
const quint32 FIT_INVALID_FLOAT32 = 0xFFFFFFFF;

// The FIT location string of the current report, in bytes including NUL.
const int FIT_LOCATION_SIZE = 15;

// FIT global message number for WEATHER_CONDITIONS, from the FIT profile.
const quint16 FIT_MESSAGE_WEATHER_CONDITIONS = 128;

// FIT WeatherReport ordinals.
const quint8 REPORT_CURRENT = 0;
const quint8 REPORT_HOURLY_FORECAST = 1;
const quint8 REPORT_DAILY_FORECAST = 2;

// Local message types. FIT only registers a local type once a record is added
// under it, so every report keeps its own type and an empty report is simply
// never defined. Same order Gadgetbridge allocates them in.
const quint8 LOCAL_TYPE_CURRENT = 0;
const quint8 LOCAL_TYPE_HOURLY = 1;
const quint8 LOCAL_TYPE_DAILY = 2;

struct WeatherField {
    quint8 number;
    quint8 size;
    quint8 baseType;
};

// Field order only has to match between a definition and the records that
// follow it, because FIT data records are positional, not tagged. The orders
// below copy Gadgetbridge exactly: its builder keeps fields in setter call
// order, so a real Gadgetbridge capture diffs field for field against ours.
//
// Fields we have no value for are kept in the definition and written as FIT's
// invalid value, so the watch shows "no data" for them. Gadgetbridge does the
// same for fields its own provider lacks.
const WeatherField CURRENT_FIELDS[] = {
    { 0, 1, FIT_BASE_ENUM }, // weather_report
    { 253, 4, FIT_BASE_UINT32 }, // timestamp
    { 9, 4, FIT_BASE_UINT32 }, // observed_at_time
    { 1, 1, FIT_BASE_SINT8 }, // temperature
    { 14, 1, FIT_BASE_SINT8 }, // low_temperature
    { 13, 1, FIT_BASE_SINT8 }, // high_temperature
    { 2, 1, FIT_BASE_ENUM }, // condition
    { 3, 2, FIT_BASE_UINT16 }, // wind_direction
    { 5, 1, FIT_BASE_UINT8 }, // precipitation_probability
    { 4, 2, FIT_BASE_UINT16 }, // wind_speed
    { 6, 1, FIT_BASE_SINT8 }, // temperature_feels_like
    { 7, 1, FIT_BASE_UINT8 }, // relative_humidity
    { 10, 4, FIT_BASE_SINT32 }, // observed_location_lat
    { 11, 4, FIT_BASE_SINT32 }, // observed_location_long
    { 17, 1, FIT_BASE_ENUM }, // air_quality
    { 15, 1, FIT_BASE_SINT8 }, // dew_point
    { 8, FIT_LOCATION_SIZE, FIT_BASE_STRING }, // location
};

const WeatherField HOURLY_FIELDS[] = {
    { 0, 1, FIT_BASE_ENUM }, // weather_report
    { 253, 4, FIT_BASE_UINT32 }, // timestamp
    { 1, 1, FIT_BASE_SINT8 }, // temperature
    { 2, 1, FIT_BASE_ENUM }, // condition
    { 3, 2, FIT_BASE_UINT16 }, // wind_direction
    { 4, 2, FIT_BASE_UINT16 }, // wind_speed
    { 5, 1, FIT_BASE_UINT8 }, // precipitation_probability
    { 6, 1, FIT_BASE_SINT8 }, // temperature_feels_like
    { 7, 1, FIT_BASE_UINT8 }, // relative_humidity
    { 15, 1, FIT_BASE_SINT8 }, // dew_point
    { 16, 4, FIT_BASE_FLOAT32 }, // uv_index
    { 17, 1, FIT_BASE_ENUM }, // air_quality
    { 18, 4, FIT_BASE_UINT32 }, // atmospheric_pressure
};

const WeatherField DAILY_FIELDS[] = {
    { 0, 1, FIT_BASE_ENUM }, // weather_report
    { 253, 4, FIT_BASE_UINT32 }, // timestamp
    { 14, 1, FIT_BASE_SINT8 }, // low_temperature
    { 13, 1, FIT_BASE_SINT8 }, // high_temperature
    { 2, 1, FIT_BASE_ENUM }, // condition
    { 5, 1, FIT_BASE_UINT8 }, // precipitation_probability
    { 12, 1, FIT_BASE_ENUM }, // day_of_week
    { 17, 1, FIT_BASE_ENUM }, // air_quality
    { 7, 1, FIT_BASE_UINT8 }, // relative_humidity
    { 4, 2, FIT_BASE_UINT16 }, // wind_speed
    { 3, 2, FIT_BASE_UINT16 }, // wind_direction
    { 16, 4, FIT_BASE_FLOAT32 }, // uv_index
    { 18, 4, FIT_BASE_UINT32 }, // atmospheric_pressure
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

// Gadgetbridge builds its weather records with ByteOrder.BIG_ENDIAN, and the
// watches render those. Keep this encoder big endian to match it byte for byte.
static inline void writeU16be(QByteArray& out, quint16 v)
{
    out.append(char(v >> 8));
    out.append(char(v & 0xFF));
}

static inline void writeU32be(QByteArray& out, quint32 v)
{
    out.append(char(v >> 24));
    out.append(char((v >> 16) & 0xFF));
    out.append(char((v >> 8) & 0xFF));
    out.append(char(v & 0xFF));
}

static inline void appendRecordDefinition(QByteArray& payload, quint8 localType, const WeatherField* fields, int fieldCount)
{
    payload.append(char(0x40 | localType)); // definition header for this local type
    payload.append(char(0)); // reserved
    payload.append(char(1)); // architecture: 1 means big endian, like Gadgetbridge
    writeU16be(payload, FIT_MESSAGE_WEATHER_CONDITIONS);
    payload.append(char(fieldCount));
    for (int i = 0; i < fieldCount; ++i) {
        payload.append(char(fields[i].number));
        payload.append(char(fields[i].size));
        payload.append(char(fields[i].baseType));
    }
}

// A FIT STRING: UTF-8 truncated to size-1 bytes, NUL, then padded with the
// STRING invalid fill 0x00. Truncation can cut a multi-byte character in
// half; Gadgetbridge does the same, so keep it byte-identical.
static inline void appendLocation(QByteArray& payload, const QString& name)
{
    QByteArray utf8 = name.toUtf8();
    if (utf8.size() > FIT_LOCATION_SIZE - 1) {
        utf8.truncate(FIT_LOCATION_SIZE - 1);
    }
    payload.append(utf8);
    payload.append(QByteArray(FIT_LOCATION_SIZE - utf8.size(), '\0'));
}

static inline void appendCurrentRecord(QByteArray& payload, const GarminWeatherSnapshot& snapshot)
{
    const quint32 timestamp = toGarminEpoch(snapshot.timestampUnix);

    payload.append(char(LOCAL_TYPE_CURRENT));
    payload.append(char(REPORT_CURRENT));
    writeU32be(payload, timestamp); // timestamp
    writeU32be(payload, timestamp); // observed_at_time
    writeTemperatureC(payload, snapshot.temperatureC);
    writeTemperatureC(payload, snapshot.minTempC);
    writeTemperatureC(payload, snapshot.maxTempC);
    payload.append(char(snapshot.conditionCode));
    writeU16be(payload, quint16(normalizeDegrees(snapshot.windDirectionDeg)));
    writePercent(payload, snapshot.precipitationProbabilityPercent);
    writeU16be(payload, windSpeedMillimetresPerSecond(snapshot.windSpeedMs));
    writeTemperatureC(payload, snapshot.temperatureC); // feels_like: no data, use temperature
    writePercent(payload, snapshot.humidityPercent);
    writeU32be(payload, quint32(degreesToSemicircles(snapshot.latitude)));
    writeU32be(payload, quint32(degreesToSemicircles(snapshot.longitude)));
    payload.append(char(FIT_INVALID_ENUM)); // air_quality: Amazfish has no AQI
    payload.append(char(FIT_INVALID_SINT8)); // dew_point: no data
    appendLocation(payload, snapshot.locationName);
}

// The hourly report is the same global message as the other two, selected by
// field 0. GarminWeatherMessage builds one record per hour by interpolating the
// three-hourly entries the API returns, see hourlyRecord().
static inline void appendHourlyRecord(QByteArray& payload, const GarminWeatherHour& hour)
{
    payload.append(char(LOCAL_TYPE_HOURLY));
    payload.append(char(REPORT_HOURLY_FORECAST));
    writeU32be(payload, toGarminEpoch(hour.timestampUnix));
    writeTemperatureC(payload, hour.temperatureC);
    payload.append(char(hour.conditionCode));
    writeU16be(payload, quint16(normalizeDegrees(hour.windDirectionDeg)));
    writeU16be(payload, windSpeedMillimetresPerSecond(hour.windSpeedMs));
    writePercent(payload, hour.precipitationProbabilityPercent);
    writeTemperatureC(payload, hour.temperatureC); // feels_like: same stand-in Gadgetbridge uses
    writePercent(payload, hour.humidityPercent);
    payload.append(char(FIT_INVALID_SINT8)); // dew_point: no data
    writeU32be(payload, FIT_INVALID_FLOAT32); // uv_index: no data
    payload.append(char(FIT_INVALID_ENUM)); // air_quality: Amazfish has no AQI
    // atmospheric_pressure: CurrentWeather::Forecast keeps pressure in a
    // uint8_t, which cannot hold real hPa values, so send no value.
    writeU32be(payload, FIT_INVALID_UINT32);
}

// Every daily record carries the time the data was taken, not the time of that
// day. day_of_week is what tells the days apart.
static inline void appendDailyRecord(QByteArray& payload, const GarminWeatherDay& day, qint64 takenAtUnix)
{
    payload.append(char(LOCAL_TYPE_DAILY));
    payload.append(char(REPORT_DAILY_FORECAST));
    writeU32be(payload, toGarminEpoch(takenAtUnix));
    writeTemperatureC(payload, day.minTempC);
    writeTemperatureC(payload, day.maxTempC);
    payload.append(char(day.conditionCode));
    writePercent(payload, day.precipitationProbabilityPercent);
    payload.append(char(quint8(day.dayOfWeek)));
    payload.append(char(FIT_INVALID_ENUM)); // air_quality: Amazfish has no AQI
    writePercent(payload, day.humidityPercent);
    writeU16be(payload, windSpeedMillimetresPerSecond(day.windSpeedMs));
    writeU16be(payload, quint16(normalizeDegrees(day.windDirectionDeg)));
    writeU32be(payload, FIT_INVALID_FLOAT32); // uv_index: no data
    writeU32be(payload, FIT_INVALID_UINT32); // atmospheric_pressure: see hourly record
}

}

QByteArray FitWeatherEncoder::buildDefinitionPayload(const GarminWeatherSnapshot& snapshot)
{
    QByteArray payload;
    appendRecordDefinition(payload, LOCAL_TYPE_CURRENT, CURRENT_FIELDS, int(sizeof(CURRENT_FIELDS) / sizeof(CURRENT_FIELDS[0])));
    if (!snapshot.hours.isEmpty()) {
        appendRecordDefinition(payload, LOCAL_TYPE_HOURLY, HOURLY_FIELDS, int(sizeof(HOURLY_FIELDS) / sizeof(HOURLY_FIELDS[0])));
    }
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
    for (int i = 0; i < snapshot.hours.size(); ++i) {
        appendHourlyRecord(payload, snapshot.hours.at(i));
    }
    for (int i = 0; i < snapshot.days.size(); ++i) {
        appendDailyRecord(payload, snapshot.days.at(i), snapshot.timestampUnix);
    }
    return payload;
}
