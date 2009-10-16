/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "filterdata.h"

#include <akonadi/filter/program.h>

class FilterData::Private : public QSharedData
{
public:
    Private()
        : m_program( 0 )
    {
    }

    Private( const QString& id, const QString& name, const QList<KRss::Feed::Id>& sourceFeeds,
             Akonadi::Filter::Program* program )
        : m_id( id ), m_name( name ), m_sourceFeeds( sourceFeeds ), m_program( program )
    {
    }

    Private( const Private& other )
        : QSharedData( other ), m_id( other.m_id ), m_name( other.m_name ),
          m_sourceFeeds( other.m_sourceFeeds ), m_program( other.m_program )
    {
    }

    QString m_id;
    QString m_name;
    QList<KRss::Feed::Id> m_sourceFeeds;
    Akonadi::Filter::Program* m_program;
};

FilterData::FilterData()
    : d( new FilterData::Private )
{
}

FilterData::FilterData( const QString& id, const QString& name, const QList<KRss::Feed::Id>& sourceFeeds,
               Akonadi::Filter::Program* program )
    : d( new FilterData::Private( id, name, sourceFeeds, program ) )
{
}

FilterData::FilterData( const FilterData& other )
    : d( other.d )
{
}

FilterData::~FilterData()
{
}

void FilterData::swap( FilterData& other )
{
    qSwap( d, other.d );
}

FilterData& FilterData::operator=( const FilterData& other )
{
    FilterData copy( other );
    swap( copy );
    return *this;
}

bool FilterData::operator==( const FilterData& other ) const
{
    return d->m_id == other.d->m_id;
}

bool FilterData::operator!=( const FilterData& other ) const
{
    return !( *this == other );
}

bool FilterData::operator<( const FilterData& other ) const
{
    return d->m_id < other.d->m_id;
}

QString FilterData::id() const
{
    return d->m_id;
}

void FilterData::setId( const QString& id )
{
    d->m_id = id;
}

QString FilterData::name() const
{
    return d->m_name;
}

void FilterData::setName( const QString& name )
{
    d->m_name = name;
}

QList<KRss::Feed::Id> FilterData::sourceFeeds() const
{
    return d->m_sourceFeeds;
}

void FilterData::setSourceFeeds( const QList<KRss::Feed::Id>& sourceFeeds )
{
    d->m_sourceFeeds = sourceFeeds;
}

Akonadi::Filter::Program* FilterData::program() const
{
    return d->m_program;
}

void FilterData::setProgram( Akonadi::Filter::Program* program )
{
    d->m_program = program;
}
