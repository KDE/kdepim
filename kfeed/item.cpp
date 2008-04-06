
/*
 * This file is part of the kfeed library
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

#include "item.h"

#include "category.h"
#include "enclosure.h"
#include "person.h"

#include <KDateTime>

#include <QHash>
#include <QList>
#include <QString>

#include <algorithm>

using namespace KFeed;

class Item::Private : public QSharedData
{
public:
    Private() : status( Read ), hash( 0 ), idIsHash( false ), commentsCount( -1 ), feedId( -1 )
    {}

    Private( const Private& other );
    
    bool operator!=( const Private& other ) const
    {
        return !( *this == other );
    }

    bool operator==( const Private& other ) const
    {
        return status == other.status 
            && hash == other.hash 
            && idIsHash == other.idIsHash 
            && id == other.id && title == other.title
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
            && feedId == other.feedId;
    }
    
    int status;
    int hash;
    bool idIsHash;
    QString id;
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
};

Item::Private::Private( const Private& other )
    : QSharedData( other ),
    status( other.status ),
    hash( other.hash ),
    idIsHash( other.idIsHash ),
    id( other.id ),
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
    feedId( other.feedId )
{
}

Item::Item() : d( new Private )
{
}

Item::~Item()
{
}

void Item::swap( Item& other )
{
    std::swap( d, other.d );
}

Item& Item::operator=( const Item& other )
{
    Item copy( other );
    swap( copy );
    return *this;
}

bool Item::operator==( const Item& other ) const
{
    return *d == *(other.d);
}

bool Item::operator!=( const Item& other ) const
{
    return *d != *(other.d);
}

Item::Item( const Item& other ) : d( other.d )
{
}

QString Item::title() const
{
    return d->title;
}

void Item::setTitle( const QString& title ) 
{
    d->title = title;
}

QString Item::description() const
{
    return d->description;
}

void Item::setDescription( const QString& description ) 
{
    d->description = description;
}

QString Item::link() const
{
    return d->link;
}

void Item::setLink( const QString& link ) 
{
    d->link = link;
}

QString Item::content() const
{
    return d->content.isNull() ? d->description : d->content;
}

void Item::setContent( const QString& content ) 
{
    d->content = content;
}

KDateTime Item::datePublished() const
{
    return d->datePublished;
}

void Item::setDatePublished( const KDateTime& datePublished ) 
{
    d->datePublished = datePublished;
}

KDateTime Item::dateUpdated() const
{
    return d->dateUpdated.isValid() ? d->dateUpdated : d->datePublished;
}

void Item::setDateUpdated( const KDateTime& dateUpdated ) 
{
    d->dateUpdated = dateUpdated;
}

QString Item::id() const
{
    return d->id;
}

void Item::setId( const QString& id ) 
{
    d->id = id;
}

QList<Person> Item::authors() const
{
    return d->authors;
}

void Item::setAuthors( const QList<Person>& authors ) 
{
    d->authors = authors;
}

QList<Category> Item::categories() const
{
    return d->categories;
}

void Item::setCategories( const QList<Category>& categories ) 
{
    d->categories = categories;
}

QList<Enclosure> Item::enclosures() const
{
    return d->enclosures;
}

void Item::setEnclosures( const QList<Enclosure>& enclosures ) 
{
    d->enclosures = enclosures;
}

QString Item::language() const
{
    return d->language;
}

void Item::setLanguage( const QString& language )
{
    d->language = language;
}

int Item::commentsCount() const
{
    return d->commentsCount;
}

void Item::setCommentsCount( int commentsCount )
{
    d->commentsCount = commentsCount;
}

QString Item::commentsLink() const
{
    return d->commentsLink;
}

void Item::setCommentsLink( const QString& commentsLink )
{
    d->commentsLink = commentsLink;
}

QString Item::commentsFeed() const
{
    return d->commentsFeed;
}

void Item::setCommentsFeed( const QString& commentsFeed )
{
    d->commentsFeed = commentsFeed;
}

QString Item::commentPostUri() const
{
    return d->commentPostUri;
}

void Item::setCommentPostUri( const QString& commentPostUri )
{
    d->commentPostUri = commentPostUri;
}

bool Item::idIsHash() const
{
    return d->idIsHash;
}

void Item::setIdIsHash( bool idIsHash )
{
    d->idIsHash = idIsHash;
}

int Item::hash() const
{
    return d->hash;
}

void Item::setHash( int hash )
{
    d->hash = hash;
}

int Item::status() const
{
    return d->status;
}

void Item::setStatus( int status )
{
    d->status = status;
}

int Item::sourceFeedId() const
{
    return d->feedId;
}

void Item::setSourceFeedId( int feedId )
{
    d->feedId = feedId;
}

QHash<QString, QString> Item::customProperties() const
{
    return d->customProperties;
}

QString Item::customProperty( const QString& key ) const
{
    return d->customProperties.value( key );
}

void Item::setCustomProperty( const QString& key, const QString& value )
{
    d->customProperties.insert( key, value );
}

