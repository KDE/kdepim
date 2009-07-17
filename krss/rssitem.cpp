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

#include "rssitem.h"
#include "category.h"
#include "enclosure.h"
#include "person.h"

#include <KDateTime>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>

#include <algorithm>

using namespace KRss;

class RssItem::Private : public QSharedData
{
public:
    Private() : hash( 0 ), guidIsHash( false ), commentsCount( -1 ), feedId( -1 ),
        headersLoaded( false ), contentLoaded( false )
    {}

    Private( const Private& other );

    bool operator!=( const Private& other ) const
    {
        return !( *this == other );
    }

    bool operator==( const Private& other ) const
    {
        return hash == other.hash
            && guidIsHash == other.guidIsHash
            && guid == other.guid && title == other.title
            && link == other.link
            && description == other.description
            && content == other.content
            && datePublished == other.datePublished
            && dateUpdated == other.dateUpdated
            && authors == other.authors
            && enclosures == other.enclosures
            && categories == other.categories
            && language == other.language
            && commentsCount == other.commentsCount
            && commentsLink == other.commentsLink
            && commentsFeed == other.commentsFeed
            && commentPostUri == other.commentPostUri
            && customProperties == other.customProperties
            && feedId == other.feedId
            && headersLoaded == other.headersLoaded
            && contentLoaded == other.contentLoaded;
    }

    int hash;
    bool guidIsHash;
    QString guid;
    QString title;
    QString link;
    QString description;
    QString content;
    KDateTime datePublished;
    KDateTime dateUpdated;
    QList<Person> authors;
    QList<Enclosure> enclosures;
    QList<Category> categories;
    QString language;
    int commentsCount;
    QString commentsLink;
    QString commentsFeed;
    QString commentPostUri;
    QHash<QString, QString> customProperties;
    int feedId;
    bool headersLoaded;
    bool contentLoaded;
};

RssItem::Private::Private( const Private& other )
    : QSharedData( other ),
    hash( other.hash ),
    guidIsHash( other.guidIsHash ),
    guid( other.guid ),
    title( other.title ),
    link( other.link ),
    description( other.description ),
    content( other.content ),
    datePublished( other.datePublished ),
    dateUpdated( other.dateUpdated ),
    authors( other.authors ),
    enclosures( other.enclosures ),
    categories( other.categories ),
    language( other.language ),
    commentsCount( other.commentsCount ),
    commentsLink( other.commentsLink ),
    commentPostUri( other.commentPostUri ),
    customProperties( other.customProperties ),
    feedId( other.feedId ),
    headersLoaded( other.headersLoaded ),
    contentLoaded( other.contentLoaded )
{
}

QByteArray RssItem::flagNew()
{
    return "\\New";
}

QByteArray RssItem::flagRead()
{
    return "\\Seen";
}

QByteArray RssItem::flagImportant()
{
    return "\\Important";
}

QByteArray RssItem::flagDeleted()
{
    return "\\Deleted";
}

ItemId RssItem::itemIdFromAkonadi( const Akonadi::Item::Id& id )
{
    return id;
}

Akonadi::Item::Id RssItem::itemIdToAkonadi( const ItemId& itemId )
{
    return itemId;
}

RssItem::RssItem() : d( new Private )
{
}

RssItem::~RssItem()
{
}

void RssItem::swap( RssItem& other )
{
    std::swap( d, other.d );
}

RssItem& RssItem::operator=( const RssItem& other )
{
    RssItem copy( other );
    swap( copy );
    return *this;
}

bool RssItem::operator==( const RssItem& other ) const
{
    return *d == *(other.d);
}

bool RssItem::operator!=( const RssItem& other ) const
{
    return *d != *(other.d);
}

RssItem::RssItem( const RssItem& other ) : d( other.d )
{
}

bool RssItem::headersLoaded() const
{
    return d->headersLoaded;
}

void RssItem::setHeadersLoaded( bool headersLoaded )
{
    d->headersLoaded = headersLoaded;
}

bool RssItem::contentLoaded() const
{
    return d->contentLoaded;
}

void RssItem::setContentLoaded( bool contentLoaded )
{
    d->contentLoaded = contentLoaded;
}

QString RssItem::title() const
{
    return d->title;
}

void RssItem::setTitle( const QString& title )
{
    d->title = title;
}

QString RssItem::description() const
{
    return d->description;
}

void RssItem::setDescription( const QString& description )
{
    d->description = description;
}

QString RssItem::link() const
{
    return d->link;
}

void RssItem::setLink( const QString& link )
{
    d->link = link;
}

QString RssItem::content() const
{
    return d->content;
}

void RssItem::setContent( const QString& content )
{
    d->content = content;
}

KDateTime RssItem::datePublished() const
{
    return d->datePublished;
}

void RssItem::setDatePublished( const KDateTime& datePublished )
{
    d->datePublished = datePublished;
}

KDateTime RssItem::dateUpdated() const
{
    return d->dateUpdated;
}

void RssItem::setDateUpdated( const KDateTime& dateUpdated )
{
    d->dateUpdated = dateUpdated;
}

QString RssItem::guid() const
{
    return d->guid;
}

void RssItem::setGuid( const QString& guid )
{
    d->guid = guid;
}

QList<Person> RssItem::authors() const
{
    return d->authors;
}

void RssItem::setAuthors( const QList<Person>& authors )
{
    d->authors = authors;
}

QList<Category> RssItem::categories() const
{
    return d->categories;
}

void RssItem::setCategories( const QList<Category>& categories )
{
    d->categories = categories;
}

QList<Enclosure> RssItem::enclosures() const
{
    return d->enclosures;
}

void RssItem::setEnclosures( const QList<Enclosure>& enclosures )
{
    d->enclosures = enclosures;
}

QString RssItem::language() const
{
    return d->language;
}

void RssItem::setLanguage( const QString& language )
{
    d->language = language;
}

int RssItem::commentsCount() const
{
    return d->commentsCount;
}

void RssItem::setCommentsCount( int commentsCount )
{
    d->commentsCount = commentsCount;
}

QString RssItem::commentsLink() const
{
    return d->commentsLink;
}

void RssItem::setCommentsLink( const QString& commentsLink )
{
    d->commentsLink = commentsLink;
}

QString RssItem::commentsFeed() const
{
    return d->commentsFeed;
}

void RssItem::setCommentsFeed( const QString& commentsFeed )
{
    d->commentsFeed = commentsFeed;
}

QString RssItem::commentPostUri() const
{
    return d->commentPostUri;
}

void RssItem::setCommentPostUri( const QString& commentPostUri )
{
    d->commentPostUri = commentPostUri;
}

bool RssItem::guidIsHash() const
{
    return d->guidIsHash;
}

void RssItem::setGuidIsHash( bool guidIsHash )
{
    d->guidIsHash = guidIsHash;
}

int RssItem::hash() const
{
    return d->hash;
}

void RssItem::setHash( int hash )
{
    d->hash = hash;
}

int RssItem::sourceFeedId() const
{
    return d->feedId;
}

void RssItem::setSourceFeedId( int feedId )
{
    d->feedId = feedId;
}

QHash<QString, QString> RssItem::customProperties() const
{
    return d->customProperties;
}

QString RssItem::customProperty( const QString& key ) const
{
    return d->customProperties.value( key );
}

void RssItem::setCustomProperty( const QString& key, const QString& value )
{
    d->customProperties.insert( key, value );
}
