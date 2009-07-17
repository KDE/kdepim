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

#include "item.h"
#include "item_p.h"
#include "rssitem.h"
#include "tagidsattribute.h"
#include "category.h"
#include "enclosure.h"
#include "person.h"

#include <syndication/tools.h>

#include <KDateTime>

#include <algorithm>

using namespace KRss;

const char* Item::HeadersPart = "RssHeaders";
const char* Item::ContentPart = "RssContent";

// static
QString Item::mimeType()
{
    return "application/rss+xml";
}

Item::Item() : d( new Private )
{
}

Item::Item( const Item& other ) : d( other.d )
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

bool Item::operator<( const Item& other ) const
{
    return *d < *(other.d);
}

ItemId Item::id() const
{
    return RssItem::itemIdFromAkonadi( d->akonadiItem.id() );
}

bool Item::isNull() const
{
    //PENDING(frank) review
    return d->akonadiItem.id() == -1;
}

QList<TagId> Item::tags() const
{
    const TagIdsAttribute * const attr = d->akonadiItem.attribute<TagIdsAttribute>();
    if ( attr )
        return attr->tagIds();

    return QList<TagId>();
}

void Item::setTags( const QList<TagId>& tags )
{
    d->akonadiItem.attribute<TagIdsAttribute>( Akonadi::Item::AddIfMissing )->setTagIds( tags );
}

QString Item::title() const
{
    return d->akonadiItem.payload<RssItem>().title();
}

QString Item::titleAsPlainText() const
{
    if ( d->titleAsPlainText.isNull() )
        d->titleAsPlainText = Syndication::htmlToPlainText( title() );
    return d->titleAsPlainText;
}

void Item::setTitle( const QString& title )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setTitle( title );
    d->akonadiItem.setPayload<RssItem>( payload );
    d->titleAsPlainText.clear();
}

QString Item::description() const
{
    return d->akonadiItem.payload<RssItem>().description();
}

void Item::setDescription( const QString& description )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setDescription( description );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::link() const
{
    return d->akonadiItem.payload<RssItem>().link();
}

void Item::setLink( const QString& link )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setLink( link );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::content() const
{
    const RssItem payload = d->akonadiItem.payload<RssItem>();
    if ( payload.content().isEmpty() && d->akonadiItem.loadedPayloadParts().contains( ContentPart ) ) {
        return payload.description();
    }

    return payload.content();
}

void Item::setContent( const QString& content )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setContent( content );
    d->akonadiItem.setPayload<RssItem>( payload );
}

KDateTime Item::datePublished() const
{
    return d->akonadiItem.payload<RssItem>().datePublished();
}

void Item::setDatePublished( const KDateTime& datePublished )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setDatePublished( datePublished );
    d->akonadiItem.setPayload<RssItem>( payload );
}

KDateTime Item::dateUpdated() const
{
    const RssItem payload = d->akonadiItem.payload<RssItem>();
    return payload.dateUpdated().isValid() ? payload.dateUpdated() : payload.datePublished();
}

void Item::setDateUpdated( const KDateTime& dateUpdated )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setDateUpdated( dateUpdated );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::guid() const
{
    return d->akonadiItem.payload<RssItem>().guid();
}

void Item::setGuid( const QString& guid )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setGuid( guid );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QList<Person> Item::authors() const
{
    return d->akonadiItem.payload<RssItem>().authors();
}

void Item::setAuthors( const QList<Person>& authors )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setAuthors( authors );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QList<Category> Item::categories() const
{
    return d->akonadiItem.payload<RssItem>().categories();
}

void Item::setCategories( const QList<Category>& categories )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCategories( categories );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QList<Enclosure> Item::enclosures() const
{
    return d->akonadiItem.payload<RssItem>().enclosures();
}

void Item::setEnclosures( const QList<Enclosure>& enclosures )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setEnclosures( enclosures );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::language() const
{
    return d->akonadiItem.payload<RssItem>().language();
}

void Item::setLanguage( const QString& language )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setLanguage( language );
    d->akonadiItem.setPayload<RssItem>( payload );
}

int Item::commentsCount() const
{
    return d->akonadiItem.payload<RssItem>().commentsCount();
}

void Item::setCommentsCount( int commentsCount )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCommentsCount( commentsCount );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::commentsLink() const
{
    return d->akonadiItem.payload<RssItem>().commentsLink();
}

void Item::setCommentsLink( const QString& commentsLink )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCommentsLink( commentsLink );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::commentsFeed() const
{
    return d->akonadiItem.payload<RssItem>().commentsFeed();
}

void Item::setCommentsFeed( const QString& commentsFeed )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCommentsFeed( commentsFeed );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QString Item::commentPostUri() const
{
    return d->akonadiItem.payload<RssItem>().commentPostUri();
}

void Item::setCommentPostUri( const QString& commentPostUri )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCommentPostUri( commentPostUri );
    d->akonadiItem.setPayload<RssItem>( payload );
}

bool Item::guidIsHash() const
{
    return d->akonadiItem.payload<RssItem>().guidIsHash();
}

void Item::setGuidIsHash( bool guidIsHash )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setGuidIsHash( guidIsHash );
    d->akonadiItem.setPayload<RssItem>( payload );
}

int Item::hash() const
{
    return d->akonadiItem.payload<RssItem>().hash();
}

void Item::setHash( int hash )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setHash( hash );
    d->akonadiItem.setPayload<RssItem>( payload );
}

Item::Status Item::status() const
{
    //PENDING(frank) this looks like a candidate for caching
    Status stat;
    if ( d->akonadiItem.hasFlag( RssItem::flagNew() ) )
        stat |= Item::New;

    if ( !d->akonadiItem.hasFlag( RssItem::flagRead() ) )
        stat |= Item::Unread;

    if ( d->akonadiItem.hasFlag( RssItem::flagImportant() ) )
        stat |= Item::Important;

    if ( d->akonadiItem.hasFlag( RssItem::flagDeleted() ) )
        stat |= Item::Deleted;

    return stat;
}

bool Item::isImportant() const
{
    return d->akonadiItem.hasFlag( RssItem::flagImportant() );
}

bool Item::isRead() const
{
    return d->akonadiItem.hasFlag( RssItem::flagRead() );
}

bool Item::isUnread() const
{
    return !isRead();
}

bool Item::isNew() const
{
    return d->akonadiItem.hasFlag( RssItem::flagNew() );
}

bool Item::isDeleted() const
{
    return d->akonadiItem.hasFlag( RssItem::flagDeleted() );
}

void Item::setStatus( const Item::Status& stat )
{
    Akonadi::Item::Flags flags;
    if ( stat.testFlag( Item::New ) )
        flags.insert( RssItem::flagNew() );

    if ( !stat.testFlag( Item::Unread ) )
        flags.insert( RssItem::flagRead() );

    if ( stat.testFlag( Item::Important ) )
        flags.insert( RssItem::flagImportant() );

    if ( stat.testFlag( Item::Deleted ) )
        flags.insert( RssItem::flagDeleted() );

    d->akonadiItem.setFlags( flags );
}

int Item::sourceFeedId() const
{
    return d->akonadiItem.payload<RssItem>().sourceFeedId();
}

void Item::setSourceFeedId( int feedId )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setSourceFeedId( feedId );
    d->akonadiItem.setPayload<RssItem>( payload );
}

QHash<QString, QString> Item::customProperties() const
{
    return d->akonadiItem.payload<RssItem>().customProperties();
}

QString Item::customProperty( const QString& key ) const
{
    return d->akonadiItem.payload<RssItem>().customProperty( key );
}

void Item::setCustomProperty( const QString& key, const QString& value )
{
    RssItem payload = d->akonadiItem.payload<RssItem>();
    payload.setCustomProperty( key, value );
    d->akonadiItem.setPayload<RssItem>( payload );
}
