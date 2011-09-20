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

#include "feedcollection.h"
#include "feedpropertiescollectionattribute.h"
#
#include <akonadi/cachepolicy.h>

using Akonadi::Collection;
using namespace KRss;


FeedCollection::FeedCollection()
{
}

FeedCollection::FeedCollection( const Akonadi::Collection &collection )
    : Collection( collection )
{
}

Akonadi::Collection FeedCollection::findFolder( const Akonadi::Collection& c ) {
    if ( FeedCollection( c ).isFolder() )
        return c;
    else
        return c.parentCollection();
}

bool FeedCollection::isFolder() const
{
    return contentMimeTypes() == QStringList( Akonadi::Collection::mimeType() );
}

QString FeedCollection::title() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->name();

    return QString();
}

void FeedCollection::setTitle( const QString &title )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setName( title );
}

QString FeedCollection::xmlUrl() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->xmlUrl();

    return QString();
}

void FeedCollection::setXmlUrl( const QString &xmlUrl )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setXmlUrl( xmlUrl );
}

QString FeedCollection::htmlUrl() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->htmlUrl();

    return QString();
}

void FeedCollection::setHtmlUrl( const QString &htmlUrl )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setHtmlUrl( htmlUrl );
}

QString FeedCollection::description() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->description();

    return QString();
}

void FeedCollection::setDescription( const QString &description )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setDescription( description );
}

QString FeedCollection::feedType() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->feedType();

    return QString();
}

void FeedCollection::setFeedType( const QString &feedType )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setFeedType( feedType );
}

bool FeedCollection::preferItemLinkForDisplay() const
{
    FeedPropertiesCollectionAttribute *attr = attribute<FeedPropertiesCollectionAttribute>();
    if ( attr )
        return attr->preferItemLinkForDisplay();

    return false;
}

void FeedCollection::setPreferItemLinkForDisplay( bool b )
{
    attribute<FeedPropertiesCollectionAttribute>( AddIfMissing )->setPreferItemLinkForDisplay( b );
}

int FeedCollection::fetchInterval() const
{
    return cachePolicy().intervalCheckTime();
}

void FeedCollection::setFetchInterval( int interval )
{
    Akonadi::CachePolicy policy = cachePolicy();
    policy.setInheritFromParent( false );
    policy.setIntervalCheckTime( interval );
    setCachePolicy( policy );
}
