/*
    Copyright (C) 2008    Dmitry Ivanov <vonami@gmail.com>

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

#include "tag_p.h"
#include "krss/config-nepomuk.h"

#ifdef HAVE_NEPOMUK
#include "nepomuktagprovider/nepomuktag_p.h"
#else
#include "defaulttagprovider/defaulttag_p.h"
#endif

using namespace boost;
using namespace KRss;

QString Tag::idToString( const TagId& id )
{
    return id.url();
}

TagId Tag::idFromString( const QString& s )
{
    return TagId( s );
}

Tag::Tag( const TagId& id )
    : d( new TagPrivateImpl( id ) )
{
}

Tag::Tag( const Tag& other )
    : d( other.d )
{
}

Tag::~Tag()
{
}

void Tag::swap( Tag& other )
{
    qSwap( d, other.d );
}

Tag& Tag::operator=( const Tag& other )
{
    Tag copy( other );
    swap( copy );
    return *this;
}

bool Tag::operator==( const Tag& other ) const
{
    return d->m_id == other.d->m_id;
}

bool Tag::operator!=( const Tag& other ) const
{
    return !( *this == other );
}

bool Tag::operator<( const Tag& other ) const
{
    return d->asTuple() < other.d->asTuple();
}

QString Tag::label() const
{
    return d->label();
}

void Tag::setLabel( const QString &label )
{
    d->setLabel( label );
}

QString Tag::description() const
{
    return d->description();
}

void Tag::setDescription( const QString &description )
{
    d->setDescription( description );
}

QString Tag::icon() const
{
    return d->icon();
}

void Tag::setIcon( const QString &icon )
{
    d->setIcon( icon );
}

TagId Tag::id() const
{
    return d->m_id;
}

bool Tag::isNull() const
{
    return d->m_id.isEmpty();
}
