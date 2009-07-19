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

#include "resource.h"
#include "resource_p.h"

using namespace KRss;

ResourcePrivate::~ResourcePrivate()
{
}

Resource::Resource( const QString& resourceId, const QString& name, QObject* parent )
    : QObject( parent ), d_ptr( new ResourcePrivate( resourceId, name ) )
{
}

Resource::Resource( ResourcePrivate& dd, QObject* parent )
    : QObject( parent ), d_ptr( &dd )
{
}

Resource::~Resource()
{
    delete d_ptr;
}

QString Resource::id() const
{
    return d_ptr->m_id;
}

QString Resource::name() const
{
    return d_ptr->m_name;
}

void Resource::registerListeningFeed( Feed* feed )
{
    d_ptr->m_feeds.insert( feed->id(), QPointer<Feed>( feed ) );
}

void Resource::unregisterListeningFeed( Feed* feed )
{
    d_ptr->m_feeds.remove( feed->id() );
}

void Resource::triggerFeedChanged( const KRss::Feed::Id& feedId )
{
    emit feedChanged( d_ptr->m_id, feedId );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerChanged();
}

void Resource::triggerFeedRemoved( const KRss::Feed::Id& feedId )
{
    emit feedRemoved( d_ptr->m_id, feedId );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerRemoved();
}

void Resource::triggerStatisticsChanged( const KRss::Feed::Id& feedId,
                                         const Akonadi::CollectionStatistics &statistics )
{
    emit statisticsChanged( d_ptr->m_id, feedId, statistics );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerStatisticsChanged( statistics );
}

void Resource::triggerFetchStarted( const KRss::Feed::Id& feedId )
{
    emit fetchStarted( d_ptr->m_id, feedId );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerFetchStarted();
}

void Resource::triggerFetchPercent( const KRss::Feed::Id& feedId, uint percentage )
{
    emit fetchPercent( d_ptr->m_id, feedId, percentage );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerFetchPercent( percentage );
}

void Resource::triggerFetchFinished( const KRss::Feed::Id& feedId )
{
    emit fetchFinished( d_ptr->m_id, feedId );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerFetchFinished();
}

void Resource::triggerFetchFailed( const KRss::Feed::Id& feedId, const QString &errorMessage )
{
    emit fetchFailed( d_ptr->m_id, feedId, errorMessage );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerFetchFailed( errorMessage );
}

void Resource::triggerFetchAborted( const KRss::Feed::Id& feedId )
{
    emit fetchAborted( d_ptr->m_id, feedId );
    const QPointer<Feed> feed = d_ptr->m_feeds.value( feedId );
    if ( feed )
        feed->triggerFetchAborted();
}

#include "resource.moc"
