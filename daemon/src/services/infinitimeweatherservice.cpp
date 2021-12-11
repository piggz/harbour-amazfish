#include "infinitimeweatherservice.h"
#include "typeconversion.h"

#include <QDebug>

const char* InfiniTimeWeatherService::UUID_SERVICE_WEATHER = "00040000-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeWeatherService::UUID_CHARACTERISTIC_WEATHER_DATA = "00040001-78fc-48fe-8e23-433b3a1942d0";
const char* InfiniTimeWeatherService::UUID_CHARACTERISTIC_WEATHER_CONTROL = "00040002-78fc-48fe-8e23-433b3a1942d0";

InfiniTimeWeatherService::InfiniTimeWeatherService(const QString &path, QObject *parent) : QBLEService(UUID_SERVICE_WEATHER, path, parent)
{
    qDebug() << Q_FUNC_INFO;
    connect(this, &QBLEService::characteristicChanged, this, &InfiniTimeWeatherService::characteristicChanged);
}

void InfiniTimeWeatherService::sendWeather(CurrentWeather *weather)
{


#if 0
    HuamiDevice *device = qobject_cast<HuamiDevice*>(parent());

    if (!device || device->softwareRevision() > "V0.0.8.74") { //Lexical string comparison should be fine here
        supportsConditionString = true;
    }

    char condition = HuamiWeatherCondition::mapToAmazfitBipWeatherCode(weather->weatherCode());

    //int tz_offset_hours = SimpleTimeZone.getDefault().getOffset(weather->timestamp * 1000L) / (1000 * 60 * 60);
    int tz_offset_hours = 0; //!TODO work out what to do with this

    qDebug() << "Sending condition";
    QByteArray buf;
    QBuffer buffer(&buf);
    buffer.open(QIODevice::WriteOnly);

    char temp = weather->temperature() - 273;
    qint32 dt = qToLittleEndian(weather->dateTime());

    qDebug() << dt << temp << condition;

    buffer.putChar(char(0x02));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(condition);
    buffer.putChar(temp);
    if (supportsConditionString) {
        buffer.write(weather->description().toLatin1());
        buffer.putChar((char)0x00);
    }

    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);
    buf.clear();
    buffer.close();

    qDebug() << "Sending aqi";
    buffer.open(QIODevice::WriteOnly);
    buffer.putChar(char(0x04));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(char(0x00)); //No AQI data, so just write 0
    buffer.putChar(char(0x00));
    if (supportsConditionString) {
        buffer.write(QString("(n/a)").toLatin1());
        buffer.putChar((char)0x00);
    }
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);

    qDebug() << "Sending forecast";

    buf.clear();
    char NR_DAYS = (char) (qMin(weather->forecastCount(), 6) + 1);

    buffer.open(QIODevice::WriteOnly);

    dt = qToLittleEndian(weather->dateTime());

    qDebug() << dt << temp << condition;

    buffer.putChar(char(0x01));
    buffer.write((char*)&dt, sizeof(qint32));
    buffer.putChar(char(tz_offset_hours * 4));
    buffer.putChar(NR_DAYS);
    buffer.putChar(condition);
    buffer.putChar(condition);
    buffer.putChar((char) (weather->maxTemperature() - 273));
    buffer.putChar((char) (weather->minTemperature() - 273));
    if (supportsConditionString) {
        buffer.write(weather->description().toLatin1());
        buffer.putChar((char)0x00);
    }

    for (int f = 0; f < NR_DAYS - 1; f++) {
        CurrentWeather::Forecast fc = weather->forecast(f);
        qDebug() << "Forecast:" << f << fc.weatherCode() << fc.maxTemperature() << fc.minTemperature();

        condition = HuamiWeatherCondition::mapToAmazfitBipWeatherCode(fc.weatherCode());

        buffer.putChar(condition);
        buffer.putChar(condition);
        buffer.putChar((char) (fc.maxTemperature() - 273));
        buffer.putChar((char) (fc.minTemperature() - 273));

        if (supportsConditionString) {
            buffer.write(fc.description().toLatin1());
            buffer.putChar((char)0x00);
        }
    }
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, buf);

    //Send forecast location
    QByteArray location;
    location += (char)(0x08);
    location += weather->city()->name().toLocal8Bit();
    location += (char)(0x00);
    writeChunked(UUID_CHARACTERISTIC_MIBAND_CHUNKED_TRANSFER, 1, location);
#endif
}
