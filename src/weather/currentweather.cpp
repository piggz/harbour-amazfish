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

int CurrentWeather::temperature() const
{
    return m_temperature;
}

void CurrentWeather::refresh()
{
    qDebug() << "Refreshing weather";

    clear();

    if (!m_city) {
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

    QJsonObject main = object.value("main").toObject();
    m_temperature = int(main.value("temp").toDouble());
    m_minTemperature = int(main.value("temp_min").toDouble());
    m_maxTemperature = int(main.value("temp_max").toDouble());
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
    foreach (const QJsonValue & value, list) {
        QJsonObject obj = value.toObject();

        qlonglong dt = obj.value("dt").toVariant().toLongLong() * 1000L;

        QDate d = QDateTime::fromMSecsSinceEpoch(dt).date();
        QTime t = QDateTime::fromMSecsSinceEpoch(dt).time();

        qDebug() << "Forecast on " << dt << d << t;

        if (d > last_day && t >= QTime(12,0)) {
            last_day = d;

            QJsonObject weather = obj.value("weather").toArray().first().toObject();

            int code = weather.value("id").toVariant().toInt();
            QString desc = weather.value("description").toVariant().toString();

            QJsonObject main = obj.value("main").toObject();
            int min_temp = int(main.value("temp_min").toDouble());
            int max_temp = int(main.value("temp_max").toDouble());

            Forecast f;
            f.setDateTime(dt);
            f.setDescription(desc);
            f.setMaxTemperature(max_temp);
            f.setMinTemperature(min_temp);
            f.setWeatherCode(code);

            qDebug() << "Forecast:" << dt << desc << max_temp << min_temp << code;

            m_forecasts << f;
        }
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
    foreach (QString key, arguments.keys()) {
        query.addQueryItem(key, arguments.value(key).toString());
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

int CurrentWeather::minTemperature() const
{
    return m_minTemperature;
}

int CurrentWeather::maxTemperature() const
{
    return m_maxTemperature;
}

int CurrentWeather::dateTime() const
{
    return m_dateTime;
}

QString CurrentWeather::description() const
{
    return m_description;
}

int CurrentWeather::Forecast::minTemperature() const
{
    return m_minTemperature;
}

void CurrentWeather::Forecast::setMinTemperature(int minTemperature)
{
    m_minTemperature = minTemperature;
}

int CurrentWeather::Forecast::maxTemperature() const
{
    return m_maxTemperature;
}

void CurrentWeather::Forecast::setMaxTemperature(int maxTemperature)
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

void CurrentWeather::Forecast::setDateTime(int dateTime)
{
    m_dateTime = dateTime;
}

QString CurrentWeather::Forecast::description() const
{
    return m_description;
}

int CurrentWeather::Forecast::dateTime() const
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
