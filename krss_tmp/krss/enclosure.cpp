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

#include "enclosure.h"

#include <QString>

#include <algorithm>

using namespace KRss;

class Enclosure::Private : public QSharedData
{
public:
    Private() : length( 0 ), duration( 0 )
    {
    }

    Private( const Private& other );

    ~Private();

    bool operator==( const Private& other ) const
    {
        return url == other.url && title == other.title && type == other.type && length == other.length && duration == other.duration;
    }

    bool operator!=( const Private& other ) const
    {
        return url != other.url && title != other.title && type != other.type && length != other.length && duration != other.duration;
    }

    bool isNull() const
    {
        return url.isNull() && title.isNull() && type.isNull() && length == 0 && duration == 0;
    }

    QString url;
    QString title;
    QString type;
    uint length;
    uint duration;
};

Enclosure::Private::Private( const Private& other )
    : QSharedData( other ),
    url( other.url ),
    title( other.title ),
    type( other.type ),
    length( other.length ),
    duration( other.duration )
{
}

Enclosure::Private::~Private() {}

Enclosure::Enclosure() : d( new Private )
{
}

Enclosure::Enclosure( const Enclosure& other ) : d( other.d )
{
}

Enclosure::~Enclosure()
{
}

bool Enclosure::isNull() const
{
    return d->isNull();
}

void Enclosure::swap( Enclosure& other )
{
    std::swap( d, other.d );
}

Enclosure& Enclosure::operator=( const Enclosure& other )
{
    Enclosure copy( other );
    swap( copy );
    return *this;
}

bool Enclosure::operator==( const Enclosure& other ) const
{
    return *d == *(other.d);
}


bool Enclosure::operator!=( const Enclosure& other ) const
{
    return *d != *(other.d);
}

QString Enclosure::url() const
{
    return d->url;
}

void Enclosure::setUrl( const QString& url )
{
    d->url = url;
}

QString Enclosure::title() const
{
    return d->title;
}

void Enclosure::setTitle( const QString& title )
{
    d->title = title;
}

QString Enclosure::type() const
{
    return d->type;
}

void Enclosure::setType( const QString& type )
{
    d->type = type;
}

uint Enclosure::length() const
{
    return d->length;
}

void Enclosure::setLength( uint length )
{
    d->length = length;
}

uint Enclosure::duration() const
{
    return d->duration;
}

void Enclosure::setDuration( uint duration )
{
    d->duration = duration;
}

