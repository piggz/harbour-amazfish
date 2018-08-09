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

class CurrentWeatherModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(City * city READ city WRITE setCity NOTIFY cityChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString temperature READ temperature NOTIFY temperatureChanged)
public:
    explicit CurrentWeatherModel(QObject *parent=0);
    City * city() const;
    void setCity(City *city);
    QString icon() const;
    QString temperature() const;
    
    QString language() const;
    void setLanguage(const QString &language);
    
signals:
    void cityChanged();
    void iconChanged();
    void temperatureChanged();
    void languageChanged();
public slots:
    void refresh();

private:
    void handleFinished(const QByteArray &reply);
    static QString getIconFromCode(int code);
    
    void request(const QString &connection, const QVariantMap &arguments);
    virtual void clear();

    QNetworkAccessManager *network;
    QNetworkReply *m_reply;
    City *m_city;
    QString m_icon;
    QString m_temperature;
    QString m_language;
};

#endif // CURRENTWEATHERMODEL_H
