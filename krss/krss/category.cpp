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

#include "category.h"

#include <QString>

#include <algorithm>

using namespace KRss;

class Category::Private : public QSharedData
{
public:
    Private();
    Private( const Private& other );
    ~Private();

    bool operator==( const Category::Private& other ) const
    {
        return term == other.term && scheme == other.scheme && label == other.label;
    }

    bool operator!=( const Category::Private& other ) const
    {
        return term != other.term && scheme != other.scheme && label != other.label;
    }

    QString term;
    QString scheme;
    QString label;
};

Category::Private::Private() {}

Category::Private::Private( const Private& other )
    : QSharedData( other ),
    term( other.term ),
    scheme( other.scheme ),
    label( other.label )
{
}

Category::Private::~Private() {}


Category::Category() : d( new Category::Private )
{
}

Category::Category( const Category& other) : d( other.d )
{
}

Category::~Category()
{
}

Category& Category::operator=( const Category& other )
{
    Category copy( other );
    swap( copy );
    return *this;
}

void Category::swap( Category& other )
{
    std::swap( d, other.d );
}

bool Category::operator==( const Category& other ) const
{
    return *d == *(other.d);
}

bool Category::operator!=( const Category& other ) const
{
    return *d != *(other.d);
}

QString Category::term() const
{
    return d->term;
}

void Category::setTerm( const QString& term )
{
    d->term = term;
}

QString Category::scheme() const
{
    return d->scheme;
}

void Category::setScheme( const QString& scheme )
{
    d->scheme = scheme;
}

QString Category::label() const
{
    return d->label;
}

void Category::setLabel( const QString& label )
{
    d->label = label;
}

