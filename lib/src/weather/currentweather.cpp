/*
 * Copyright (C) 2013 Lucien XU <sfietkonstantin@free.fr>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * The names of its contributors may not be used to endorse or promote
 *     products derived from this software without specific prior written
 *     permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

#include "currentweather.h"
#include "apikey.h"

#include <QtCore/QDebug>
#include <QtCore/QTime>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

CurrentWeather::CurrentWeather(QObject *parent) :
    QObject(parent)
{    
    network = new QNetworkAccessManager(this);
}

City * CurrentWeather::city() const
{
    return m_city;
}

void CurrentWeather::setCity(City *city)
{
    if (m_city != city) {
        m_city = city;
    }
}

qreal CurrentWeather::temperature() const
{
    return m_temperature;
}

void CurrentWeather::refresh()
{
    qDebug() << "Refreshing weather";

    clear();

    if (!m_city) {
        qDebug() << "No city set!";
        return;
    }

    qDebug() << "...city is" << m_city->name();

    QVariantMap arguments;
    arguments.insert(QLatin1String("id"), m_city->identifier());
    request(QLatin1String("weather"), arguments);
}

void CurrentWeather::clear()
{
    m_temperature = 0;
    m_weatherCode = 0;
    m_minTemperature = 0;
    m_maxTemperature = 0;
    m_description = "";
}

void CurrentWeather::handleCurrent(const QByteArray &reply)
{
    qDebug() << "Current weather data:\n" << reply;

    QJsonDocument document = QJsonDocument::fromJson(reply);
    if (document.isNull()) {
        return;
    }

    QJsonObject object = document.object();

    m_dateTime = object.value("dt").toVariant().toInt();

    QJsonObject weather = object.value("weather").toArray().first().toObject();
    m_weatherCode = weather.value("id").toVariant().toInt();
    m_description = weather.value("description").toVariant().toString();
    m_weatherIcon = weather.value("icon").toVariant().toString();

    QJsonObject wind = object.value("wind").toArray().first().toObject();
    m_windDeg = wind.value("wind_deg").toVariant().toDouble();
    m_windSpeed = wind.value("wind_speed").toVariant().toDouble();
    m_windGusts = wind.value("wind_gusts").toVariant().toDouble();

    QJsonObject clouds = object.value("clouds").toArray().first().toObject();
    m_clouds = clouds.value("all").toVariant().toInt();

    QJsonObject main = object.value("main").toObject();
    m_temperature = main.value("temp").toDouble();
    m_minTemperature = main.value("temp_min").toDouble();
    m_maxTemperature = main.value("temp_max").toDouble();
    m_humidity = main.value("humidity").toDouble();
}

void CurrentWeather::handleForecast(const QByteArray &reply)
{
    qDebug() << "Forecast weather data:\n" << reply;

    QJsonDocument document = QJsonDocument::fromJson(reply);
    if (document.isNull()) {
        return;
    }

    m_forecasts.clear();

    QJsonObject object = document.object();
    QJsonArray list = object.value("list").toArray();

    QDate last_day = QDate::currentDate();

    Forecast f;
    f.setMaxTemperature(0);
    f.setMinTemperature(500);
    f.setWindMaxSpeed(0);
    f.setWindMinSpeed(255);

    qDebug() << object;
    int total_rain = 0;
    int total_snow = 0;

    foreach (const QJsonValue & value, list) {
        QJsonObject obj = value.toObject();

        qlonglong dt = obj.value("dt").toVariant().toLongLong();

        QDate d = QDateTime::fromMSecsSinceEpoch(dt * 1000L).date();
        QTime t = QDateTime::fromMSecsSinceEpoch(dt * 1000L).time();

        QJsonObject weather = obj.value("weather").toArray().first().toObject();

        QString weatherIcon = weather.value("icon").toVariant().toString();

        int code = weather.value("id").toVariant().toInt();
        QString desc = weather.value("description").toVariant().toString();

        QJsonObject main = obj.value("main").toObject();
        int min_temp = main.value("temp_min").toDouble();
        int max_temp = main.value("temp_max").toDouble();
        int wind_speed = 0;
        f.setWindMaxSpeed(0);
        f.setWindMinSpeed(255);
        if (obj.contains("rain")) {
            QJsonObject rain = obj.value("rain").toObject();
            total_rain += rain.value("3h").toDouble();
        }
        if (obj.contains("snow")) {
            QJsonObject snow = obj.value("snow").toObject();
            total_snow += snow.value("3h").toDouble();
        }

        if (obj.contains("wind")) {
            QJsonObject wind = obj.value("wind").toObject();
            wind_speed += wind.value("speed").toDouble();
        }

        qDebug() << "Forecast on " << dt << d << t << min_temp << max_temp << desc << code << "Hour:" << t.hour();

        if (d > last_day) {
            if (last_day != QDate::currentDate()) {
                f.setRainMMDay(total_rain);
                f.setSnowMMDay(total_snow);
                m_forecasts << f;
            }
            last_day = d;
            f.setMaxTemperature(0);
            f.setMinTemperature(500);
            f.setWindMaxSpeed(0);
            f.setWindMinSpeed(255);
            total_rain = 0;
            total_snow = 0;
        }

        if (t.hour() >=12 && t.hour() < 15) { //Set the general weather description for that around mid-day
            qDebug() << "Midday" << main.value("pressure") << main.value("pressure").toDouble();
            f.setDateTime(dt);
            f.setDescription(desc);
            f.setWeatherCode(code);
            f.setWeatherIcon(weatherIcon);
            f.setPressure(main.value("pressure").toDouble());
            f.setHumidity(main.value("humidity").toDouble());
        }
        if (max_temp > f.maxTemperature()){
            f.setMaxTemperature(max_temp);
        }
        if (min_temp < f.minTemperature()) {
            f.setMinTemperature(min_temp);
        }
        if (wind_speed > f.windMaxSpeed()) {
            f.setWindMaxSpeed(wind_speed);
        }
        if (wind_speed < f.windMinSpeed()) {
            f.setWindMinSpeed(wind_speed);
        }
    }

    m_forecasts << f;

    foreach(Forecast fc, m_forecasts) {
        qDebug() << fc.dateTime() << fc.description() << fc.minTemperature() << fc.maxTemperature() << fc.weatherCode();
    }

    emit ready();
}

void CurrentWeather::request(const QString &connection, const QVariantMap &arguments)
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;
    }

    //    if (!checkValidity(connection, arguments)) {
    //        return;
    //    }

    QUrl url (QString(QLatin1String("http://api.openweathermap.org/data/2.5/%1")).arg(connection));
    QUrlQuery query;
    auto end = arguments.cend();
    for (auto it = arguments.cbegin(); it != end; ++it)
    {
        query.addQueryItem(it.key(), it.value().toString());
    }

    query.addQueryItem(QLatin1String("APPID"), API_KEY);
    query.addQueryItem(QLatin1String("mode"), QLatin1String("json"));
    query.addQueryItem(QLatin1String("lang"), m_language);
    //query.addQueryItem(QLatin1String("units"), unitString(m_unit));
    url.setQuery(query);

    m_reply = network->get(QNetworkRequest(url));

    if (connection == "weather") {
        connect(m_reply, &QNetworkReply::finished, this, &CurrentWeather::slotFinishedCurrent);
    } else if (connection == "forecast") {
        connect(m_reply, &QNetworkReply::finished, this, &CurrentWeather::slotFinishedForecast);
    }
}

QString CurrentWeather::language() const
{
    return m_language;
}

void CurrentWeather::setLanguage(const QString &language)
{
    if (m_language != language) {
        m_language = language;
    }
}

void CurrentWeather::slotFinishedCurrent()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    handleCurrent(m_reply->readAll());
    m_reply->deleteLater();
    m_reply = 0;

    //Get forecast
    QVariantMap arguments;
    arguments.insert(QLatin1String("id"), m_city->identifier());

    request(QLatin1String("forecast"), arguments);
}

void CurrentWeather::slotFinishedForecast()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    handleForecast(m_reply->readAll());
    m_reply->deleteLater();
    m_reply = 0;
}

int CurrentWeather::weatherCode() const
{
    return m_weatherCode;
}

QString CurrentWeather::weatherIcon() const
{
    return m_weatherIcon;
}

qreal CurrentWeather::windDeg() const
{
    return m_windDeg;
}

qreal CurrentWeather::windSpeed() const
{
    return m_windSpeed;
}

qreal CurrentWeather::windGusts() const
{
    return m_windGusts;
}

int CurrentWeather::humidity() const
{
    return m_humidity;
}

int CurrentWeather::clouds() const
{
    return m_clouds;
}

qreal CurrentWeather::minTemperature() const
{
    return m_minTemperature;
}

qreal CurrentWeather::maxTemperature() const
{
    return m_maxTemperature;
}

qlonglong CurrentWeather::dateTime() const
{
    return m_dateTime;
}

QString CurrentWeather::description() const
{
    return m_description;
}

qreal CurrentWeather::Forecast::minTemperature() const
{
    return m_minTemperature;
}

void CurrentWeather::Forecast::setMinTemperature(qreal minTemperature)
{
    m_minTemperature = minTemperature;
}

qreal CurrentWeather::Forecast::maxTemperature() const
{
    return m_maxTemperature;
}

void CurrentWeather::Forecast::setWeatherIcon(QString _weatherIcon)
{
    m_weatherIcon = _weatherIcon;
}

QString CurrentWeather::Forecast::weatherIcon() const
{
    return m_weatherIcon;
}


void CurrentWeather::Forecast::setMaxTemperature(qreal maxTemperature)
{
    m_maxTemperature = maxTemperature;
}

int CurrentWeather::Forecast::weatherCode() const
{
    return m_weatherCode;
}

void CurrentWeather::Forecast::setWeatherCode(int weatherCode)
{
    m_weatherCode = weatherCode;
}

void CurrentWeather::Forecast::setDescription(const QString &description)
{
    m_description = description;
}

void CurrentWeather::Forecast::setDateTime(qlonglong dateTime)
{
    m_dateTime = dateTime;
}

uint8_t CurrentWeather::Forecast::rainMMDay() const
{
    return m_rainMMDay;
}

void CurrentWeather::Forecast::setRainMMDay(uint8_t newRainMMDay)
{
    m_rainMMDay = newRainMMDay;
}

uint8_t CurrentWeather::Forecast::snowMMDay() const
{
    return m_snowMMDay;
}

void CurrentWeather::Forecast::setSnowMMDay(uint8_t newSnowMMDay)
{
    m_snowMMDay = newSnowMMDay;
}

uint8_t CurrentWeather::Forecast::clouds() const
{
    return m_clouds;
}

void CurrentWeather::Forecast::setClouds(uint8_t newClouds)
{
    m_clouds = newClouds;
}

uint8_t CurrentWeather::Forecast::humidity() const
{
    return m_humidity;
}

void CurrentWeather::Forecast::setHumidity(uint8_t newHumidity)
{
    m_humidity = newHumidity;
}

uint8_t CurrentWeather::Forecast::pressure() const
{
    return m_pressure;
}

void CurrentWeather::Forecast::setPressure(uint8_t newPressure)
{
    m_pressure = newPressure;
}

uint8_t CurrentWeather::Forecast::windDirection() const
{
    return m_windDirection;
}

void CurrentWeather::Forecast::setWindDirection(uint8_t newWindDirection)
{
    m_windDirection = newWindDirection;
}

uint8_t CurrentWeather::Forecast::windMaxSpeed() const
{
    return m_windMaxSpeed;
}

void CurrentWeather::Forecast::setWindMaxSpeed(uint8_t newWindMaxSpeed)
{
    m_windMaxSpeed = newWindMaxSpeed;
}

uint8_t CurrentWeather::Forecast::windMinSpeed() const
{
    return m_windMinSpeed;
}

void CurrentWeather::Forecast::setWindMinSpeed(uint8_t newWindMinSpeed)
{
    m_windMinSpeed = newWindMinSpeed;
}

QString CurrentWeather::Forecast::description() const
{
    return m_description;
}

qlonglong CurrentWeather::Forecast::dateTime() const
{
    return m_dateTime;
}

int CurrentWeather::forecastCount() const
{
    return m_forecasts.size();
}

const CurrentWeather::Forecast& CurrentWeather::forecast(int f) const
{
    return m_forecasts.at(f);
}
