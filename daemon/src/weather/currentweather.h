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

#ifndef CURRENTWEATHERMODEL_H
#define CURRENTWEATHERMODEL_H

#include "city.h"

class QNetworkReply;
class QNetworkAccessManager;

class CurrentWeather : public QObject
{
    Q_OBJECT
public:
    explicit CurrentWeather(QObject *parent=0);

    class Forecast {

    public:
        int minTemperature() const;
        void setMinTemperature(int minTemperature);

        int maxTemperature() const;
        void setMaxTemperature(int maxTemperature);

        int weatherCode() const;
        void setWeatherCode(int weatherCode);

        void setDescription(const QString &description);
        QString description() const;

        int dateTime() const;
        void setDateTime(int dateTime);


    private:
        int m_minTemperature = 0;
        int m_maxTemperature = 0;
        int m_weatherCode = 0;
        QString m_description;
        int m_dateTime = 0;
    };


    City * city() const;
    Q_INVOKABLE void setCity(City *city);
    QString language() const;
    void setLanguage(const QString &language);

    int temperature() const;
    int minTemperature() const;
    int maxTemperature() const;
    int weatherCode() const;
    QString description() const;
    int dateTime() const;

    Q_INVOKABLE void refresh();

    int forecastCount() const;
    const Forecast& forecast(int f) const;

signals:
    void ready();

private:
    void handleCurrent(const QByteArray &reply);
    void handleForecast(const QByteArray &reply);

    Q_SLOT void slotFinishedCurrent();
    Q_SLOT void slotFinishedForecast();

    void request(const QString &connection, const QVariantMap &arguments);
    virtual void clear();

    QNetworkAccessManager *network = nullptr;
    QNetworkReply *m_reply = nullptr;
    City *m_city = nullptr;
    QString m_language;

    int m_temperature = 0;
    int m_minTemperature = 0;
    int m_maxTemperature = 0;
    int m_weatherCode = 0;
    QString m_description;
    int m_dateTime = 0;

    QList<Forecast> m_forecasts;
};

#endif // CURRENTWEATHERMODEL_H
