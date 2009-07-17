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

#ifndef KRSS_TAG_H
#define KRSS_TAG_H

#include "krss_export.h"
#include "krss/config-nepomuk.h"

#include <KUrl>

#include <QtCore/QString>
#include <QtCore/QMetaType>
#include <QtCore/QtDebug>
#include <QtCore/QSharedDataPointer>

namespace KRss {

typedef KUrl TagId;

class KRSS_EXPORT Tag
{
public:

    static QString idToString( const TagId& id );
    static TagId idFromString( const QString& s );

    explicit Tag( const TagId &uri=TagId() );
    Tag( const Tag& other );
    ~Tag();

    void swap( Tag& other );
    Tag& operator=( const Tag& other );

    bool operator==( const Tag& other ) const;
    bool operator!=( const Tag& other ) const;
    bool operator<( const Tag& other ) const;

    QString label() const;
    void setLabel( const QString& label );
    QString description() const;
    void setDescription( const QString& description );
    QString icon() const;
    void setIcon( const QString& label );

    TagId id() const;
    bool isNull() const;

private:
#ifdef HAVE_NEPOMUK
    typedef class NepomukTagPrivate TagPrivateImpl;
    QSharedDataPointer<TagPrivateImpl> d;
#else
    friend class DefaultTagProvider;
    friend class DefaultTagCreateJob;
    friend class DefaultTagModifyJob;
    friend class DefaultTagDeleteJob;
    friend class DefaultTagCreateReferencesJob;
    friend class DefaultTagModifyReferencesJob;
    friend class DefaultTagDeleteReferencesJob;
    typedef class DefaultTagPrivate TagPrivateImpl;
    QSharedDataPointer<TagPrivateImpl> d;
#endif
};

inline QDebug operator<<( QDebug debug, const Tag &tag )
{
    debug.nospace() << tag.id();
    return debug.space();
}

KRSS_EXPORT inline uint qHash( const Tag &tag )
{
    return qHash( tag.id() );
}

} //namespace KRss

Q_DECLARE_METATYPE(KRss::Tag)
Q_DECLARE_METATYPE(QList<KRss::Tag>)
Q_DECLARE_METATYPE(QList<KRss::TagId>)

#endif // KRSS_TAG_H
