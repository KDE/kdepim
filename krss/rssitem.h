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

#ifndef KRSS_RSSITEM_H
#define KRSS_RSSITEM_H

#include "krss_export.h"
#include "item.h"

#include <Akonadi/Item>
#include <QtCore/QSharedDataPointer>

class KDateTime;

template <class T> class QList;
template <class K, class T> class QHash;
class QString;

namespace KRss {

class Category;
class Enclosure;
class Person;

class KRSS_EXPORT RssItem
{
public:
    // return the corresponding Akonadi flags
    // for the status flags defined in item.h
    static QByteArray flagNew();
    static QByteArray flagRead();
    static QByteArray flagImportant();
    static QByteArray flagDeleted();

    static ItemId itemIdFromAkonadi( const Akonadi::Item::Id& id );
    static Akonadi::Item::Id itemIdToAkonadi( const ItemId& itemId );

    RssItem();
    RssItem( const RssItem& other );
    ~RssItem();

    void swap( RssItem& other );
    RssItem& operator=( const RssItem& other );
    bool operator==( const RssItem& other ) const;
    bool operator!=( const RssItem& other ) const;

    bool headersLoaded() const;
    void setHeadersLoaded( bool headersLoaded );
    bool contentLoaded() const;
    void setContentLoaded( bool contentLoaded );

    int hash() const;
    void setHash( int hash );
    bool guidIsHash() const;
    void setGuidIsHash( bool isHash );
    int sourceFeedId() const;
    void setSourceFeedId( int id );
    QString title() const;
    void setTitle( const QString& title );
    QString link() const;
    void setLink( const QString& link );
    QString description() const;
    void setDescription( const QString& description );
    QString content() const;
    void setContent( const QString& content );
    KDateTime datePublished() const;
    void setDatePublished( const KDateTime& date );
    KDateTime dateUpdated() const;
    void setDateUpdated( const KDateTime& date );
    QString guid() const;
    void setGuid( const QString& guid );
    QList<Person> authors() const;
    void setAuthors( const QList<Person>& authors );
    QString language() const;
    void setLanguage( const QString& language );
    QList<Enclosure> enclosures() const;
    void setEnclosures( const QList<Enclosure>& enclosures );
    QList<Category> categories() const;
    void setCategories( const QList<Category>& categories );
    int commentsCount() const;
    void setCommentsCount( int count );
    QString commentsLink() const;
    void setCommentsLink( const QString& link );
    QString commentsFeed() const;
    void setCommentsFeed( const QString& feed );
    QString commentPostUri() const;
    void setCommentPostUri( const QString& uri );
    QHash<QString, QString> customProperties() const;
    QString customProperty( const QString& key ) const;
    void setCustomProperty( const QString& key, const QString& value );

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace KRss

#endif // KRSS_RSSITEM_H
