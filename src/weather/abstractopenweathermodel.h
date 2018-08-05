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

#ifndef ABSTRACTOPENWEATHERMODEL_H
#define ABSTRACTOPENWEATHERMODEL_H

#include <QtCore/QAbstractListModel>
#include <QtCore/QVariantMap>

class QNetworkReply;
class QNetworkAccessManager;

class AbstractOpenWeatherModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_ENUMS(Unit)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(Unit unit READ unit WRITE setUnit NOTIFY unitChanged)
public:
    enum Status {
        Idle,
        Loading,
        Error
    };
    enum Unit {
        Metric,
        Imperial
    };
    explicit AbstractOpenWeatherModel(QObject *parent = 0);
    virtual ~AbstractOpenWeatherModel();
    int count() const;
    Status status() const;
    QString language() const;
    void setLanguage(const QString &language);
    Unit unit() const;
    void setUnit(Unit unit);
signals:
    void countChanged();
    void statusChanged();
    void languageChanged();
    void unitChanged();
protected:
    void request(const QString &connection, const QVariantMap &arguments);
    virtual void clear() = 0;
    virtual bool checkValidity(const QString &connection, const QVariantMap &arguments);
    virtual Status handleFinished(const QByteArray &reply) = 0;
    void setStatus(Status status);
    QNetworkAccessManager *network;
private:
    static QString unitString(Unit unit);
    Status m_status;
    QString m_language;
    Unit m_unit;
    QNetworkReply *m_reply;
private slots:
    void slotFinished();
};

#endif // ABSTRACTOPENWEATHERMODEL_H
