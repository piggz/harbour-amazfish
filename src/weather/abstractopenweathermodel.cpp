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

#include "abstractopenweathermodel.h"
#include "apikey.h"

#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

static const char *DEFAULT_LANGUAGE = "en";
static const char *METRIC = "metric";
static const char *IMPERIAL = "imperial";

AbstractOpenWeatherModel::AbstractOpenWeatherModel(QObject *parent) :
    QAbstractListModel(parent)
{
    m_status = Idle;
    m_language = QLatin1String(DEFAULT_LANGUAGE);
    m_unit = Metric;

    network = new QNetworkAccessManager(this);
    m_reply = 0;
}

AbstractOpenWeatherModel::~AbstractOpenWeatherModel()
{
    if (m_reply) {
        m_reply->abort();
        m_reply->deleteLater();
    }
}

int AbstractOpenWeatherModel::count() const
{
    return rowCount();
}

AbstractOpenWeatherModel::Status AbstractOpenWeatherModel::status() const
{
    return m_status;
}

QString AbstractOpenWeatherModel::language() const
{
    return m_language;
}

void AbstractOpenWeatherModel::setLanguage(const QString &language)
{
    if (m_language != language) {
        m_language = language;
        emit languageChanged();
    }
}

AbstractOpenWeatherModel::Unit AbstractOpenWeatherModel::unit() const
{
    return m_unit;
}

void AbstractOpenWeatherModel::setUnit(Unit unit)
{
    if (m_unit != unit) {
        m_unit = unit;
        emit unitChanged();
    }
}

void AbstractOpenWeatherModel::request(const QString &connection, const QVariantMap &arguments)
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = 0;
    }

    clear();
    if (!checkValidity(connection, arguments)) {
        return;
    }

    QUrl url (QString(QLatin1String("http://api.openweathermap.org/data/2.5/%1")).arg(connection));
    QUrlQuery query;
    foreach (QString key, arguments.keys()) {
        query.addQueryItem(key, arguments.value(key).toString());
    }

    query.addQueryItem(QLatin1String("APPID"), API_KEY);
    query.addQueryItem(QLatin1String("mode"), QLatin1String("json"));
    query.addQueryItem(QLatin1String("lang"), m_language);
    query.addQueryItem(QLatin1String("units"), unitString(m_unit));
    url.setQuery(query);

    m_reply = network->get(QNetworkRequest(url));
    connect(m_reply, SIGNAL(finished()), this, SLOT(slotFinished()));
    setStatus(Loading);
}

bool AbstractOpenWeatherModel::checkValidity(const QString &connection,
                                             const QVariantMap &arguments)
{
    Q_UNUSED(connection)
    Q_UNUSED(arguments)
    return true;
}

QString AbstractOpenWeatherModel::unitString(Unit unit)
{
    switch (unit) {
    case Metric:
        return QLatin1String(METRIC);
        break;
    case Imperial:
        return QLatin1String(IMPERIAL);
        break;
    default:
        break;
    }
    return QString();
}

void AbstractOpenWeatherModel::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void AbstractOpenWeatherModel::slotFinished()
{
    if (m_reply->error() != QNetworkReply::NoError) {
        setStatus(Error);
        m_reply->deleteLater();
        m_reply = 0;
        return;
    }

    setStatus(handleFinished(m_reply->readAll()));
    m_reply->deleteLater();
    m_reply = 0;
}
