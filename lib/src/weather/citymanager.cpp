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

#include "citymanager.h"
#include <QtCore/QCoreApplication>
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QFile>
#include <QtCore/QDebug>

static const char *IDENTIFIER = "identifier";
static const char *NAME = "name";
static const char *STATE = "state";
static const char *COUNTRY = "country";
static const char *COUNTRY_CODE = "country_code";
static const char *LONGITUDE = "longitude";
static const char *LATITUDE = "latitude";
static const char *PROPERTIES = "properties";

CityManager::CityManager(QObject *parent) :
    QObject(parent)
{
    loadCities();
}

QList<QObject *> CityManager::cities() const
{
    QList<QObject *> citiesObjects;
    foreach (City *city, m_cities) {
        citiesObjects.append(city);
    }

    return citiesObjects;
}

QVariantMap CityManager::properties(const QString &identifier)
{
    return m_citiesMap.value(identifier)->properties();
}

void CityManager::addCity(const QString &identifier, const QString &name, const QString &state,
                          const QString &country, const QString &countryCode,
                          float longitude, float latitude)
{
    City *city = City::create(identifier, name, state, country, countryCode,
                              longitude, latitude, this);
    m_cities.append(city);
    m_citiesMap.insert(identifier, city);
    emit citiesChanged();

    save();
}

void CityManager::removeCity(const QString &identifier)
{
    City *city = m_citiesMap.value(identifier);
    if (!city) {
        return;
    }

    m_citiesMap.remove(identifier);
    m_cities.removeAll(city);
    city->deleteLater();
    emit citiesChanged();

    save();
}

void CityManager::removeAllCities()
{    
    m_citiesMap.clear();
    qDeleteAll(m_cities.begin(), m_cities.end());
    m_cities.clear();
            
    emit citiesChanged();

    save();
}

void CityManager::loadCities()
{
    m_citiesMap.clear();
    qDeleteAll(m_cities.begin(), m_cities.end());
    m_cities.clear();

    QFile file (configFilePath());
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QJsonDocument jsonDocument = QJsonDocument::fromJson(file.readAll());
    file.close();

    QJsonArray jsonArray = jsonDocument.array();
    foreach (const QJsonValue &value, jsonArray) {
        QJsonObject jsonCity = value.toObject();
        City *city = City::create(jsonCity.value(IDENTIFIER).toString(),
                                  jsonCity.value(NAME).toString(),
                                  jsonCity.value(STATE).toString(),
                                  jsonCity.value(COUNTRY).toString(),
                                  jsonCity.value(COUNTRY_CODE).toString(),
                                  jsonCity.value(LONGITUDE).toDouble(),
                                  jsonCity.value(LATITUDE).toDouble(), this);
        QJsonObject cityProperties = jsonCity.value(PROPERTIES).toObject();
        QVariantMap properties;
        foreach (const QString &key, cityProperties.keys()) {
            properties.insert(key, cityProperties.value(key).toVariant());
        }
        city->setProperties(properties);

        m_cities.append(city);
        m_citiesMap.insert(city->identifier(), city);
    }
}

//void CityManager::addProperties(const QString &identifier, const QVariantMap &properties)
//{
//    if (!m_citiesMap.contains(identifier)) {
//        return;
//    }

//    m_citiesMap.value(identifier)->setProperties(properties);
//    save();
//}

//void CityManager::clearProperties(const QString &identifier)
//{
//    if (!m_citiesMap.contains(identifier)) {
//        return;
//    }

//    m_citiesMap.value(identifier)->setProperties(QVariantMap());
//    save();
//}

QString CityManager::configFilePath()
{
    QDir dir (QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    if (!dir.mkpath(QString("%1/%2").arg(qApp->organizationName(), qApp->applicationName()))) {
        return QString();
    }
    if (!dir.cd(qApp->organizationName())) {
        return QString();
    }
    if (!dir.cd(qApp->applicationName())) {
        return QString();
    }

    return dir.absoluteFilePath("config.json");
}

void CityManager::save()
{
    QJsonArray jsonArray;
    foreach (City *cityEntry, m_cities) {
        QJsonObject jsonCity;
        jsonCity.insert(IDENTIFIER, cityEntry->identifier());
        jsonCity.insert(NAME, cityEntry->name());
        jsonCity.insert(STATE, cityEntry->state());
        jsonCity.insert(COUNTRY, cityEntry->country());
        jsonCity.insert(COUNTRY_CODE, cityEntry->countryCode());
        jsonCity.insert(LONGITUDE, cityEntry->longitude());
        jsonCity.insert(LATITUDE, cityEntry->latitude());


        if (!cityEntry->properties().isEmpty()) {
            QJsonObject jsonCityProperties;
            foreach (const QString &key, cityEntry->properties().keys()) {
                jsonCityProperties.insert(key,
                                          QJsonValue::fromVariant(cityEntry->properties().value(key)));
            }
            jsonCity.insert(PROPERTIES, jsonCityProperties);
        }
        jsonArray.append(jsonCity);
    }

    QJsonDocument jsonDocument (jsonArray);
    QFile file (configFilePath());
    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }

    file.write(jsonDocument.toJson(QJsonDocument::Compact));
    file.close();
}
