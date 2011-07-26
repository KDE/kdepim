/*
 * This file is part of the krss library
 *
 * Copyright (C) 2007 Frank Osterfeld <osterfeld@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "person.h"

#include <QString>

#include <algorithm>

using namespace KRss;

class Person::Private : public QSharedData
{
public:

    Private() {}
    Private( const Private& other );
    ~Private() {}

    bool operator==( const Private& other ) const
    {
        return name == other.name && email == other.email && uri == other.uri;
    }

    bool operator!=( const Private& other ) const
    {
        return name != other.name && email != other.email && uri != other.uri;
    }

    QString name;
    QString email;
    QString uri;
};

Person::Private::Private( const Private& other )
    : QSharedData( other ),
    name( other.name ),
    email( other.email ),
    uri( other.uri )
{
}

QString Person::name() const
{
    return d->name;
}

void Person::setName( const QString& name )
{
    d->name = name;
}

QString Person::email() const
{
    return d->email;
}

void Person::setEmail( const QString& email )
{
    d->email = email;
}

QString Person::uri() const
{
    return d->uri;
}

void Person::setUri( const QString& uri )
{
    d->uri = uri;
}

Person::Person() : d( new Private )
{
}

Person::Person( const Person& other) : d( other.d )
{
}

Person::~Person()
{
}

void Person::swap( Person& other )
{
    std::swap( d, other.d );
}

Person& Person::operator=( const Person& other )
{
    Person copy( other );
    swap( copy );
    return *this;
}

bool Person::operator==( const Person& other ) const
{
    return *d == *(other.d);
}

bool Person::operator!=( const Person& other ) const
{
    return *d != *(other.d);
}
