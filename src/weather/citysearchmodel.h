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

#ifndef CITYSEARCHMODEL_H
#define CITYSEARCHMODEL_H

#include "abstractopenweathermodel.h"
#include <QtCore/QSet>

class CityItem;
class CitySearchModel : public AbstractOpenWeatherModel
{
    Q_OBJECT
public:
    enum CitySearchModelRole {
        IdRole,
        NameRole,
        StateRole,
        CountryRole,
        CountryCodeRole,
        LongitudeRole,
        LatitudeRole
    };
    explicit CitySearchModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
public slots:
    void search(const QString &city);
protected:
    QHash<int, QByteArray> roleNames() const;
    void clear();
    bool checkValidity(const QString &connection, const QVariantMap &arguments);
    Status handleFinished(const QByteArray &reply);
private:
    QList<CityItem *> m_cities;
    QList<CityItem *> m_resolvingCities;
    QMap<QString, CityItem *> m_resolvingCitiesMap;
    QSet<QNetworkReply *> m_resolvingReplies;
private slots:
    void slotStateResolverFinished();
};

#endif // CITYSEARCHMODEL_H
