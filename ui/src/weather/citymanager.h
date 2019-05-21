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

#ifndef CITYMANAGER_H
#define CITYMANAGER_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include "city.h"

class CityManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QList<QObject *> cities READ cities NOTIFY citiesChanged)
public:
    explicit CityManager(QObject *parent = 0);
    QList<QObject *> cities() const;
    Q_INVOKABLE QVariantMap properties(const QString &identifier);
signals:
    void citiesChanged();
public slots:
    void addCity(const QString &identifier, const QString &name, const QString &state,
                 const QString &country, const QString &countryCode,
                 float longitude, float latitude);
    void removeCity(const QString &identifier);
    void removeAllCities();
//    void addProperties(const QString &identifier, const QVariantMap &properties);
//    void clearProperties(const QString &identifier);
private:
    static QString configFilePath();
    void save();
    QList<City *> m_cities;
    QMap<QString, City *> m_citiesMap;
};

#endif // CITYMANAGER_H
