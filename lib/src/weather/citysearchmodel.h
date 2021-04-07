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

#include <QtCore/QSet>
#include <QAbstractListModel>

class QNetworkReply;
class QNetworkAccessManager;

class CityItem;
class CitySearchModel : public QAbstractListModel
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
    enum Status {
        Idle,
        Loading,
        Error
    };
    Q_ENUM(Status)

    Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged)
    explicit CitySearchModel(QObject *parent = 0);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Status status() const;
    
    public slots:
    void search(const QString &city);

signals:
    void statusChanged();
    
protected:
    void clear();
    bool checkValidity(const QString &connection, const QVariantMap &arguments);
private:    
    void request(const QString &connection, const QVariantMap &arguments);
    void setStatus(Status status);
    Q_SLOT void slotFinished();
    Status handleFinished(const QByteArray &reply);
    
    QList<CityItem *> m_cities;
    QList<CityItem *> m_resolvingCities;
    QMap<QString, CityItem *> m_resolvingCitiesMap;
    QSet<QNetworkReply *> m_resolvingReplies;
    QNetworkAccessManager *network = nullptr;
    QNetworkReply *m_reply = nullptr;
    
    
    //static QString unitString(Unit unit);
    Status m_status = Idle;
    QString m_language;
    //Unit m_unit;
private slots:
    void slotStateResolverFinished();
};

#endif // CITYSEARCHMODEL_H
