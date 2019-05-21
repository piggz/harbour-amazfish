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

#include "city.h"

City::City(QObject *parent) :
    QObject(parent)
{
}

City * City::create(const QString &identifier, const QString &name, const QString &state,
                    const QString &country, const QString &countryCode,
                    float longitude, float latitude, QObject *parent)
{
    City *city = new City(parent);
    city->m_identifier = identifier;
    city->m_name = name;
    city->m_state = state;
    city->m_country = country;
    city->m_countryCode = countryCode;
    city->m_longitude = longitude;
    city->m_latitude = latitude;
    return city;
}

QString City::identifier() const
{
    return m_identifier;
}

QString City::name() const
{
    return m_name;
}

QString City::state() const
{
    return m_state;
}

QString City::country() const
{
    return m_country;
}

QString City::countryCode() const
{
    return m_countryCode;
}

float City::longitude() const
{
    return m_longitude;
}

float City::latitude() const
{
    return m_latitude;
}

QVariantMap City::properties() const
{
    return m_properties;
}

void City::setProperties(const QVariantMap &properties)
{
    m_properties = properties;
}
