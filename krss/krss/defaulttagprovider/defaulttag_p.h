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

#ifndef KRSS_DEFAULTTAG_P_H
#define KRSS_DEFAULTTAG_P_H

#include "krss/tag_p.h"
#include "tagpropertiesattribute.h"
#include <akonadi/collection.h>

using namespace KRss;
using Akonadi::Collection;

namespace KRss {

class DefaultTagPrivate : public TagPrivate
{
public:
    explicit DefaultTagPrivate( const TagId& id )
        : TagPrivate( id )
    {
        m_collection.setRemoteId( Tag::idToString( id ) );
    }

    DefaultTagPrivate( const DefaultTagPrivate& other )
        : TagPrivate( other ), m_collection( other.m_collection )
    {
    }

    Collection akonadiCollection( ) const
    {
        return m_collection;
    }

    void setAkonadiCollection( const Collection& collection )
    {
        m_collection = collection;
        m_id = Tag::idFromString( collection.remoteId() );
    }

    QString label() const
    {
        TagPropertiesAttribute *attr = m_collection.attribute<TagPropertiesAttribute>();
        if ( attr )
            return attr->label();

        return QString();
    }

    void setLabel( const QString& label )
    {
        m_collection.attribute<TagPropertiesAttribute>( Collection::AddIfMissing )->setLabel( label );
        m_collection.setName( "rss-tag-" + label );
    }

    QString description() const
    {
        TagPropertiesAttribute *attr = m_collection.attribute<TagPropertiesAttribute>();
        if ( attr )
            return attr->description();

        return QString();
    }

    void setDescription( const QString& description )
    {
        m_collection.attribute<TagPropertiesAttribute>( Collection::AddIfMissing )->setDescription( description );
    }

    QString icon() const
    {
        TagPropertiesAttribute *attr = m_collection.attribute<TagPropertiesAttribute>();
        if ( attr )
            return attr->icon();

        return QString();
    }

    void setIcon( const QString& icon )
    {
        m_collection.attribute<TagPropertiesAttribute>( Collection::AddIfMissing )->setIcon( icon );
    }

private:
    Collection m_collection;
};

} // namespace KRss

#endif // KRSS_DEFAULTTAG_P_H
