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

#include "virtualfeedcollection.h"
#include "virtualfeedpropertiesattribute.h"
#include "tagidsattribute.h"
#include "subscriptionlabelscollectionattribute.h"

using Akonadi::Collection;
using namespace KRss;

Feed::Id VirtualFeedCollection::feedIdFromAkonadi( const Akonadi::Collection::Id& id )
{
    return id;
}

Akonadi::Collection::Id VirtualFeedCollection::feedIdToAkonadi( const Feed::Id& feedId )
{
    return feedId;
}

VirtualFeedCollection::VirtualFeedCollection()
{
}

VirtualFeedCollection::VirtualFeedCollection( const Akonadi::Collection& collection )
    : Collection( collection )
{
}

Feed::Id VirtualFeedCollection::feedId() const
{
    return feedIdFromAkonadi( id() );
}

QString VirtualFeedCollection::title() const
{
    VirtualFeedPropertiesAttribute *attr = attribute<VirtualFeedPropertiesAttribute>();
    if ( attr )
        return attr->title();

    return QString();
}

void VirtualFeedCollection::setTitle( const QString& title )
{
    attribute<VirtualFeedPropertiesAttribute>( AddIfMissing )->setTitle( title );
}

QString VirtualFeedCollection::description() const
{
    VirtualFeedPropertiesAttribute *attr = attribute<VirtualFeedPropertiesAttribute>();
    if ( attr )
        return attr->description();

    return QString();
}

void VirtualFeedCollection::setDescription( const QString& description )
{
    attribute<VirtualFeedPropertiesAttribute>( AddIfMissing )->setDescription( description );
}

QList<TagId> VirtualFeedCollection::tags() const
{
    const TagIdsAttribute* const attr = attribute<TagIdsAttribute>();
    if ( attr )
        return attr->tagIds();

    return QList<TagId>();
}

void VirtualFeedCollection::setTags( const QList<TagId> &tags )
{
    attribute<TagIdsAttribute>( AddIfMissing )->setTagIds( tags );
}

void VirtualFeedCollection::addTag( const TagId &tag )
{
    attribute<TagIdsAttribute>( AddIfMissing )->addTagId( tag );
}

void VirtualFeedCollection::removeTag( const TagId &tag )
{
    attribute<TagIdsAttribute>( AddIfMissing )->removeTagId( tag );
}

QStringList VirtualFeedCollection::subscriptionLabels() const
{
    SubscriptionLabelsCollectionAttribute *attr = attribute<SubscriptionLabelsCollectionAttribute>();
    if ( attr )
        return attr->subscriptionLabels();

    return QStringList();
}

void VirtualFeedCollection::setSubscriptionLabels( const QStringList &subscriptionLabels )
{
    attribute<SubscriptionLabelsCollectionAttribute>( AddIfMissing )->setSubscriptionLabels( subscriptionLabels );
}

void VirtualFeedCollection::addSubscriptionLabel( const QString &subscriptionLabel )
{
    attribute<SubscriptionLabelsCollectionAttribute>( AddIfMissing )->addSubscriptionLabel( subscriptionLabel );
}

void VirtualFeedCollection::removeSubscriptionLabel( const QString &subscriptionLabel )
{
    attribute<SubscriptionLabelsCollectionAttribute>( AddIfMissing )->removeSubscriptionLabel( subscriptionLabel );
}
