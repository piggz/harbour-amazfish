#include "infinitimeweatherservice.h"
#include "typeconversion.h"
#include "codec.h"

#include <QDateTime>
#include <QDebug>
#include <math.h>

const char* InfiniTimeWeatherService::UUID_SERVICE_WEATHER = "00040000-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeWeatherService::UUID_CHARACTERISTIC_WEATHER_DATA = "00040001-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeWeatherService::UUID_CHARACTERISTIC_WEATHER_CONTROL = "00040002-78fc-48fe-8e23-433b3a1942d0";

InfiniTimeWeatherService::InfiniTimeWeatherService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_WEATHER, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    enableNotification(UUID_CHARACTERISTIC_WEATHER_DATA);
    connect(this, &QBLEService::characteristicChanged, this, &InfiniTimeWeatherService::characteristicChanged);
}

void InfiniTimeWeatherService::sendWeather(CurrentWeather *weather)
{  
    Location l;
    l.eventType = eventtype::Location;
    l.timestamp = weather->dateTime();
    l.location = weather->city()->name().toStdString();
    l.expires = 86400; //1 day
    QPoint loc = degreesToEPSG3875(QPointF(weather->city()->latitude(), weather->city()->longitude()));
    l.latitude = loc.x();
    l.longitude = loc.y();
    writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, l.toCBOR());
    
    for (int f = 0; f < weather->forecastCount(); f++) {
        CurrentWeather::Forecast fc = weather->forecast(f);
        qDebug() << Q_FUNC_INFO << "Forecast:" << f << fc.dateTime()<< fc.weatherCode() <<  (fc.maxTemperature() - 273) <<  (fc.minTemperature() - 273) << fc.humidity() << fc.pressure() << fc.windMaxSpeed() << fc.clouds();
    
        Temperature t;
        t.eventType = eventtype::Temperature;
        t.timestamp = fc.dateTime();
        t.expires = 86400; //1 day
        t.temperature = (fc.maxTemperature() - 273.15) * 100;
        t.dewPoint = fc.maxTemperature() - ((100 - fc.humidity())/5);
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, t.toCBOR());
    
        Clouds c;
        c.eventType = eventtype::Clouds;
        c.timestamp = fc.dateTime();
        c.expires = 86400; //1 day
        c.amount = fc.clouds();
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, c.toCBOR());
        
        Wind w;
        w.eventType = eventtype::Clouds;
        w.timestamp = fc.dateTime();
        w.expires = 86400; //1 day   
        w.directionMax = fc.windDirection() / 255;
        w.directionMin = w.directionMax;
        w.speedMin = fc.windMinSpeed();
        w.speedMax = fc.windMaxSpeed();
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, w.toCBOR());    
        
        Precipitation p;
        p.eventType = eventtype::Precipitation;
        p.timestamp = fc.dateTime();
        p.expires = 86400; //1 day
        if (fc.snowMMDay() > 0) {
            p.type = precipitationtype::Snow;
            p.amount = fc.snowMMDay();
        } else if (fc.rainMMDay() > 0){
            p.type = precipitationtype::Rain;
            p.amount = fc.rainMMDay();
        } else {
            p.type = precipitationtype::None;
            p.amount = 0;
        }
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, p.toCBOR());

        Humidity h;
        h.eventType = eventtype::Humidity;
        h.timestamp = fc.dateTime();
        h.expires = 86400; //1 day
        h.humidity = fc.humidity();
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, h.toCBOR());

        Pressure pr;
        pr.eventType = eventtype::Pressure;
        pr.timestamp = fc.dateTime();
        pr.expires = 86400; //1 day
        pr.pressure = fc.pressure();
        writeValue(UUID_CHARACTERISTIC_WEATHER_DATA, pr.toCBOR());
    }

}

void InfiniTimeWeatherService::characteristicChanged(const QString &characteristic, const QByteArray &value)
{
    qDebug() << Q_FUNC_INFO << characteristic << value;

}

QPoint InfiniTimeWeatherService::degreesToEPSG3875(const QPointF &location)
{
    double smRadius = 6378136.98;
    double smRange = smRadius * M_PI * 2.0;
    double smLonToX = smRange / 360.0;
    double smRadiansOverDegrees = M_PI / 180.0;

    QPoint ret;
    // compute x-map-unit
    ret.setX(location.x() * smLonToX);

    double y = location.y();

    // compute y-map-unit
    if (y > 86.0)
    {
        ret.setY(smRange);
    }
    else if (y < -86.0)
    {
        ret.setY(-smRange);
    }
    else
    {
        y *= smRadiansOverDegrees;
        y = std::log(std::tan(y) + (1.0 / std::cos(y)));
        ret.setY(y * smRadius);
    }
    return ret;
}

QByteArray InfiniTimeWeatherService::Clouds::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 4);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Amount"));
    CborLite::encodeUnsigned(buffer, amount);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Obscuration::toCBOR() const
{
    return QByteArray();
}

QByteArray InfiniTimeWeatherService::Precipitation::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 5);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Type"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(type));
    CborLite::encodeText(buffer, std::string("Amount"));
    CborLite::encodeUnsigned(buffer, amount);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Wind::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 7);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("SpeedMin"));
    CborLite::encodeUnsigned(buffer, speedMin);
    CborLite::encodeText(buffer, std::string("SpeedMax"));
    CborLite::encodeUnsigned(buffer, speedMax);
    CborLite::encodeText(buffer, std::string("DirectionMin"));
    CborLite::encodeUnsigned(buffer, directionMin);
    CborLite::encodeText(buffer, std::string("DirectionMax"));
    CborLite::encodeUnsigned(buffer, directionMax);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Temperature::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 4);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Temperature"));
    CborLite::encodeInteger(buffer, temperature);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());   
}

QByteArray InfiniTimeWeatherService::Location::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 6);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Location"));
    CborLite::encodeText(buffer, location);
    CborLite::encodeText(buffer, std::string("Latitude"));
    CborLite::encodeInteger(buffer, latitude);
    CborLite::encodeText(buffer, std::string("Longitude"));
    CborLite::encodeInteger(buffer, longitude);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Humidity::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 4);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Humidity"));
    CborLite::encodeUnsigned(buffer, humidity);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Pressure::toCBOR() const
{
    std::vector<uint8_t> buffer;
    CborLite::encodeMapSize<std::vector<uint8_t>, uint8_t>(buffer, 4);
    CborLite::encodeText(buffer, std::string("EventType"));
    CborLite::encodeUnsigned(buffer, static_cast<uint8_t>(eventType));
    CborLite::encodeText(buffer, std::string("Timestamp"));
    CborLite::encodeInteger(buffer, timestamp);
    CborLite::encodeText(buffer, std::string("Expires"));
    CborLite::encodeInteger(buffer, expires);
    CborLite::encodeText(buffer, std::string("Pressure"));
    CborLite::encodeInteger(buffer, pressure);

    return QByteArray(reinterpret_cast<const char*>(buffer.data()), buffer.size());
}

QByteArray InfiniTimeWeatherService::Special::toCBOR() const
{
    return QByteArray();
}

QByteArray InfiniTimeWeatherService::AirQuality::toCBOR() const
{
    return QByteArray();
}
