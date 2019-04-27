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

#ifndef CITY_H
#define CITY_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>

class City : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString identifier READ identifier CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString state READ state CONSTANT)
    Q_PROPERTY(QString country READ country CONSTANT)
    Q_PROPERTY(QString countryCode READ countryCode CONSTANT)
    Q_PROPERTY(float longitude READ longitude CONSTANT)
    Q_PROPERTY(float latitude READ latitude CONSTANT)
public:
    explicit City(QObject *parent = 0);
    static City * create(const QString &identifier, const QString &name, const QString &state,
                         const QString &country, const QString &countryCode,
                         float longitude, float latitude, QObject *parent = 0);
    QString identifier() const;
    QString name() const;
    QString state() const;
    QString country() const;
    QString countryCode() const;
    float longitude() const;
    float latitude() const;
    QVariantMap properties() const;
    void setProperties(const QVariantMap &properties);
private:
    QString m_identifier;
    QString m_name;
    QString m_state;
    QString m_country;
    QString m_countryCode;
    float m_longitude;
    float m_latitude;
    QVariantMap m_properties;
};

#endif // CITY_H
