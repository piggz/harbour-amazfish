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

#include "citysearchmodel.h"
#include "apikey.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QDebug>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

struct CityItem
{
    QString id;
    QString name;
    QString state;
    QString country;
    QString countryCode;
    float lon;
    float lat;
};

CitySearchModel::CitySearchModel(QObject *parent) :
 QAbstractListModel(parent)
{    
    network = new QNetworkAccessManager(this);
}

int CitySearchModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_cities.count();
}

QVariant CitySearchModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row < 0 || row >= rowCount()) {
        return QVariant();
    }

    const CityItem *city = m_cities.at(row);

    switch (role) {
    case IdRole:
        return city->id;
        break;
    case NameRole:
        return city->name;
        break;
    case StateRole:
        return city->state;
        break;
    case CountryRole:
        return city->country;
        break;
    case CountryCodeRole:
        return city->countryCode;
        break;
    case LongitudeRole:
        return QString::number(city->lon);
        break;
    case LatitudeRole:
        return QString::number(city->lat);
        break;
    default:
        return QVariant();
        break;
    }
}

void CitySearchModel::search(const QString &city)
{
    QVariantMap arguments;
    arguments.insert(QLatin1String("q"), city.trimmed());
    request(QLatin1String("find"), arguments);
}

QHash<int, QByteArray> CitySearchModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles.insert(IdRole, "identifier");
    roles.insert(NameRole, "name");
    roles.insert(StateRole, "state");
    roles.insert(CountryRole, "country");
    roles.insert(CountryCodeRole, "countryCode");
    roles.insert(LongitudeRole, "longitude");
    roles.insert(LatitudeRole, "latitude");
    return roles;
}

void CitySearchModel::clear()
{
    m_resolvingReplies.clear();
    m_resolvingCitiesMap.clear();

    if (!m_cities.isEmpty()) {
        beginRemoveRows(QModelIndex(), 0, m_cities.count() - 1);
        qDeleteAll(m_cities);
        m_cities.clear();
        endRemoveRows();
    }
}

bool CitySearchModel::checkValidity(const QString &connection, const QVariantMap &arguments)
{
    Q_UNUSED(connection)
    if (arguments.value(QLatin1String("q")).toString().isEmpty()) {
        return false;
    }
    return true;
}

CitySearchModel::Status CitySearchModel::handleFinished(const QByteArray &reply)
{
    QJsonDocument document = QJsonDocument::fromJson(reply);
    if (document.isNull()) {
        return Error;
    }

    QJsonArray list = document.object().value(QLatin1String("list")).toArray();
    qDeleteAll(m_resolvingCities);
    m_resolvingCities.clear();
    m_resolvingCitiesMap.clear();

    foreach (const QJsonValue &value, list) {
        QJsonObject cityJson = value.toObject();
        QJsonObject coordinatesJson = cityJson.value(QLatin1String("coord")).toObject();
        CityItem *city = new CityItem;
        city->id = cityJson.value(QLatin1String("id")).toVariant().toString();
        city->name = cityJson.value(QLatin1String("name")).toString();
        city->lat = coordinatesJson.value(QLatin1String("lat")).toDouble();
        city->lon = coordinatesJson.value(QLatin1String("lon")).toDouble();

        // Create state resolving request
        //QUrl url ("http://open.mapquestapi.com/nominatim/v1/reverse");
        QUrl url ("https://nominatim.openstreetmap.org/reverse.php");
            
        QUrlQuery query;
        query.addQueryItem(QLatin1String("format"), QLatin1String("json"));
        query.addQueryItem(QLatin1String("zoom"), QLatin1String("6"));
        query.addQueryItem(QLatin1String("lat"), QString::number(city->lat));
        query.addQueryItem(QLatin1String("lon"), QString::number(city->lon));
        query.addQueryItem(QLatin1String("email"), QLatin1String("amazfish@piggz.co.uk"));
        
        url.setQuery(query);

        qDebug() << url;
        
        QNetworkReply *rep = network->get(QNetworkRequest(url));
        rep->setProperty("id", city->id);
        connect(rep, SIGNAL(finished()), this, SLOT(slotStateResolverFinished()));

        // Add to the resolving
        m_resolvingCities.append(city);
        m_resolvingCitiesMap.insert(city->id, city);
    }

    if (m_resolvingCities.isEmpty()) {
        return Idle;
    } else {
        return Loading;
    }
}

void CitySearchModel::slotStateResolverFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) {
        return;
    }

    if (!reply->property("id").isValid()) {
        reply->deleteLater();
        m_resolvingReplies.remove(reply);
        return;
    }

    QString id = reply->property("id").toString();

    QByteArray doc = reply->readAll();
    QJsonDocument result = QJsonDocument::fromJson(doc);
    
    qDebug() << doc;
    
    reply->deleteLater();
    m_resolvingReplies.remove(reply);

    if (!m_resolvingCitiesMap.contains(id)) {
        return;
    }

    QJsonObject address = result.object().value(QLatin1String("address")).toObject();

    CityItem *city = m_resolvingCitiesMap.value(id);
    city->state = address.value(QLatin1String("state")).toString();
    city->country = address.value(QLatin1String("country")).toString();
    city->countryCode = address.value(QLatin1String("country_code")).toString();
    m_resolvingCitiesMap.remove(id);

    if (city->name.isEmpty()) {
        QString cityName = address.value(QLatin1String("city")).toString();
        QString townName = address.value(QLatin1String("town")).toString();
        QString villageName = address.value(QLatin1String("village")).toString();

        if (!cityName.isEmpty()) {
            city->name = cityName;
        } else if (!townName.isEmpty()) {
            city->name = townName;
        } else {
            city->name = villageName;
        }
    }

    if (m_resolvingCitiesMap.isEmpty()) {
        beginInsertRows(QModelIndex(), 0, m_resolvingCities.count() - 1);
        m_cities = m_resolvingCities;
        m_resolvingCities.clear();
        endInsertRows();
        setStatus(Idle);
    }
}

void CitySearchModel::request(const QString &connection, const QVariantMap &arguments)
{
    if (m_reply) {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    clear();
    if (!checkValidity(connection, arguments)) {
        return;
    }

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
    connect(m_reply, &QNetworkReply::finished, this, &CitySearchModel::slotFinished);
    setStatus(Loading);
}

CitySearchModel::Status CitySearchModel::status() const
{
    return m_status;
}

void CitySearchModel::setStatus(Status status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged();
    }
}

void CitySearchModel::slotFinished()
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
