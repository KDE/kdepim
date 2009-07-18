/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#ifndef KRSS_TAG_P_H
#define KRSS_TAG_P_H

#include "tag.h"
#include "config-nepomuk.h"

#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#ifdef HAVE_NEPOMUK // Nepomuk specific implementation

namespace KRss {

class TagPrivate : public QSharedData
{
public:
    explicit TagPrivate( const TagId& id )
        : m_url( id )
    {
    }

    TagPrivate( const TagPrivate& other )
        : QSharedData( other ), m_url( other.m_url ), m_label( other.m_label ),
          m_description( other.m_description ), m_icon( other.m_icon )
    {
    }

    boost::tuple<TagId, QString, QString, QString> asTuple() const
    {
        return boost::make_tuple<TagId, QString, QString, QString>( m_url, m_label, m_description, m_icon );
    }

    TagId id() const
    {
        return m_url;
    }

    void setId( const TagId& id )
    {
        m_url = id;
    }

    QString label() const
    {
        return m_label;
    }

    void setLabel( const QString& label )
    {
        m_label = label;
    }

    QString description() const
    {
        return m_description;
    }

    void setDescription( const QString& description )
    {
        m_description = description;
    }

    QString icon() const
    {
        return m_icon;
    }

    void setIcon( const QString& icon )
    {
        m_icon = icon;
    }

public:
    KUrl m_url;
    QString m_label;
    QString m_description;
    QString m_icon;
};

} // namespace KRss

#else  // Akonadi specific implementation
#include "defaulttagprovider/tagpropertiesattribute.h"
#include <akonadi/collection.h>

using Akonadi::Collection;

namespace KRss
{

class TagPrivate : public QSharedData
{
    friend class DefaultTagProvider;
    friend class DefaultTagCreateJob;
    friend class DefaultTagModifyJob;
    friend class DefaultTagDeleteJob;
    friend class DefaultTagCreateReferencesJob;
    friend class DefaultTagModifyReferencesJob;
    friend class DefaultTagDeleteReferencesJob;

// Friend classes listed above should be able to access the d-pointer of a Tag
// and thus they should be friends of the Tag class.
// BUT we can't put them in tag.h because it is a public header.
// So instead we provide here two static functions to get the d-pointer of a Tag
// Yeah, I know, this looks clumsy but it does the job.
private:
    static const QSharedDataPointer<TagPrivate>& tag_d( const Tag& tag )
    {
        return tag.d;
    }

    static QSharedDataPointer<TagPrivate>& tag_d( Tag& tag )
    {
        return tag.d;
    }

public:
    explicit TagPrivate( const TagId& id )
    {
        m_collection.setRemoteId( Tag::idToString( id ) );
    }

    TagPrivate( const TagPrivate& other )
        : QSharedData( other ), m_collection( other.m_collection )
    {
    }

    boost::tuple<TagId, QString, QString, QString> asTuple() const
    {
        return boost::make_tuple<TagId, QString, QString, QString>( id(), label(), description(), icon() );
    }

    TagId id() const
    {
        return Tag::idFromString( m_collection.remoteId() );
    }

    void setId( const TagId& id )
    {
        m_collection.setRemoteId( Tag::idToString( id ) );
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

public:
    Collection m_collection;
};

} // namespace KRss

#endif // HAVE_NEPOMUK

#endif // KRSS_TAG_P_H
