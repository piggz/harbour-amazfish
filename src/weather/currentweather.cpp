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

#include "currentweathermodel.h"
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

CurrentWeatherModel::CurrentWeatherModel(QObject *parent) :
    QObject(parent)
{    
    network = new QNetworkAccessManager(this);
}

City * CurrentWeatherModel::city() const
{
    return m_city;
}

void CurrentWeatherModel::setCity(City *city)
{
    if (m_city != city) {
        m_city = city;
        emit cityChanged();
    }
}

QString CurrentWeatherModel::temperature() const
{
    return m_temperature;
}

QString CurrentWeatherModel::icon() const
{
    return m_icon;
}

void CurrentWeatherModel::refresh()
{
    if (!m_city) {
        m_icon.clear();
        m_temperature.clear();
        emit iconChanged();
        emit temperatureChanged();
        return;
    }

    QVariantMap arguments;
    arguments.insert(QLatin1String("id"), m_city->identifier());
    request(QLatin1String("weather"), arguments);
}

void CurrentWeatherModel::clear()
{
    m_icon.clear();
    m_temperature.clear();
}

void CurrentWeatherModel::handleFinished(const QByteArray &reply)
{
    QJsonDocument document = QJsonDocument::fromJson(reply);
    if (document.isNull()) {
        return;
    }

    QJsonObject object = document.object();

    QJsonObject weather = object.value("weather").toArray().first().toObject();
    int id = weather.value("id").toVariant().toInt();
    QJsonObject main = object.value("main").toObject();

    QString icon = QString("../pics//%1").arg(getIconFromCode(id));
    if (m_icon != icon) {
        m_icon = icon;
        emit iconChanged();
    }

    QString temperature = QString("%1°").arg(int (main.value("temp").toDouble()));
    if (m_temperature != temperature) {
        m_temperature = temperature;
        emit temperatureChanged();
    }
}

QString CurrentWeatherModel::getIconFromCode(int code)
{
    int hour = QTime::currentTime().hour();
    bool night = hour < 7 || hour >= 19;
    switch (code) {
    // Storms
    case 200:
    case 210:
    case 221:
    case 230:
        return !night ? QString("weather-storm-day.png") : QString("weather-storm-night.png");
        break;
    case 201:
    case 202:
    case 211:
    case 212:
    case 231:
    case 232:
        return QString("weather-storm.png");
        break;
    // Drizzle
    case 300:
    case 301:
    case 302:
    case 310:
    case 311:
    case 312:
    case 321:
        return QString("weather-showers-scattered.png");
        break;
    // Rain
    case 500:
    case 501:
        return !night ? QString("weather-showers-scattered-day.png")
                      : QString("weather-showers-scattered-night.png");
        break;
    case 502:
    case 503:
    case 504:
        return !night ? QString("weather-showers-day.png")
                      : QString("weather-showers-night.png");
        break;
    case 511:
        return QString("weather-freezing-rain");
        break;
    case 520:
        return QString("weather-showers-scattered.png");
        break;
    case 521:
    case 522:
        return QString("weather-showers.png");
        break;
    case 600:
        return QString("weather-snow-scattered");
        break;
    // Snow
    case 601:
    case 602:
        return QString("weather-snow.png");
        break;
    case 611:
        return QString("weather-snow-rain.png");
        break;
    case 621:
        return QString("weather-snow.png");
        break;
    // Atmosphere
    case 701:
    case 711:
    case 721:
    case 731:
    case 741:
        return QString("weather-mist.png");
        break;
    // Clouds
    case 800:
        return !night ? QString("weather-clear.png") : QString("weather-clear-night.png");
        break;
    case 801:
    case 802:
        return !night ? QString("weather-few-clouds.png") : QString("weather-few-clouds-night.png");
        break;
    case 803:
        return !night ? QString("weather-clouds.png") : QString("weather-clouds-night.png");
        break;
    case 804:
        return QString("weather-many-clouds.png");
        break;
    // Extreme
    case 900:
    case 901:
    case 902:
    case 903:
    case 904:
    case 905:
        return QString("weather-none-available.png");
        break;
    case 906:
        return QString("weather-hail.png");
        break;
    default:
        return QString("weather-none-available.png");
        break;
    }
}

void CurrentWeatherModel::request(const QString &connection, const QVariantMap &arguments)
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;
    }

    clear();
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
    connect(m_reply, SIGNAL(finished()), this, SLOT(slotFinished()));
}

QString CurrentWeatherModel::language() const
{
    return m_language;
}

void CurrentWeatherModel::setLanguage(const QString &language)
{
    if (m_language != language) {
        m_language = language;
        emit languageChanged();
    }
}

