/*
    Copyright (C) 2008,2009    Dmitry Ivanov <vonami@gmail.com>

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

#include "feedlist.h"
#include "feedlist_p.h"
#include "resource.h"
#include "resourcemanager.h"
#include "feed.h"
#include "netfeed.h"
#include "searchfeed.h"
#include "feedcollection.h"
#include "retrieveresourcecollectionsjob.h"

#include <akonadi/collectionfetchjob.h>

#include <KDebug>
#include <KJob>

#include <QtCore/QStringList>

#include <algorithm>
#include <memory>

#include <cassert>

using Akonadi::Collection;
using Akonadi::CollectionFetchJob;

using namespace KRss;
using boost::shared_ptr;
using boost::dynamic_pointer_cast;

// private implementation
void FeedListPrivate::appendResourceCollections( const QList<Collection>& collections,
                                                 const shared_ptr<Resource>& resource )
{
    Q_FOREACH( const Collection& collection, collections )
        appendFeedCollection( collection, resource );

    m_resources.insert( resource->id(), resource );
    QObject::connect( resource.get(), SIGNAL( feedAdded( const QString&, const KRss::Feed::Id& ) ),
                      q, SLOT( slotFeedAdded( const QString&, const KRss::Feed::Id& ) ) );
}

void FeedListPrivate::appendFeedCollection( const FeedCollection& collection,
                                            const shared_ptr<Resource>& resource )
{
    // TODO: support for SearchFeeds
    shared_ptr<Feed> feed( new NetFeed( collection, resource ) );
    m_feeds.insert( feed->id(), feed );

    QObject::connect( feed.get(), SIGNAL( unreadCountChanged( const KRss::Feed::Id&, int ) ),
                      q, SIGNAL( unreadCountChanged( const KRss::Feed::Id&, int ) ) );
    QObject::connect( feed.get(), SIGNAL( totalCountChanged( const KRss::Feed::Id&, int ) ),
                      q, SIGNAL( totalCountChanged( const KRss::Feed::Id&, int ) ) );

    QObject::connect( feed.get(), SIGNAL( fetchStarted( const KRss::Feed::Id& ) ),
                      q, SIGNAL( fetchStarted( const KRss::Feed::Id& ) ) );
    QObject::connect( feed.get(), SIGNAL( fetchPercent( const KRss::Feed::Id& , uint ) ),
                      q, SIGNAL( fetchPercent( const KRss::Feed::Id&, uint ) ) );
    QObject::connect( feed.get(), SIGNAL( fetchFinished( const KRss::Feed::Id& ) ),
                      q, SIGNAL( fetchFinished( const KRss::Feed::Id& ) ) );
    QObject::connect( feed.get(), SIGNAL( fetchFailed( const KRss::Feed::Id&, QString ) ),
                      q, SIGNAL( fetchFailed( const KRss::Feed::Id&, QString ) ) );
    QObject::connect( feed.get(), SIGNAL( changed( const KRss::Feed::Id& ) ),
                      q, SIGNAL( feedChanged( const KRss::Feed::Id& ) ) );
    QObject::connect( feed.get(), SIGNAL( removed( const KRss::Feed::Id& ) ),
                      q, SLOT( slotFeedRemoved( const KRss::Feed::Id& ) ) );
}

void FeedListPrivate::slotFeedAdded( const QString& resourceId, const KRss::Feed::Id& id )
{
    // NetFeedCreateJob may have already added this feed to the list
    if ( m_feeds.contains( id ) )
        return;

    CollectionFetchJob *job = new CollectionFetchJob( Collection( FeedCollection::feedIdToAkonadi( id ) ),
                                                      CollectionFetchJob::Base, q );
    QObject::connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotCollectionLoadDone( KJob* ) ) );
    m_pendingJobs.insert( job, resourceId );
    job->start();
}

void FeedListPrivate::slotFeedRemoved( const KRss::Feed::Id& id )
{
    Q_UNUSED( id )
    //TODO: implement
}

void FeedListPrivate::slotCollectionLoadDone( KJob *job )
{
    if ( job->error() ) {
        kWarning() << "Failed to load collection from a resource";
        kWarning() << job->errorString();
        return;
    }

    const CollectionFetchJob* const fjob = qobject_cast<const CollectionFetchJob*>( job );
    assert( fjob );
    assert( fjob->collections().count() == 1 );
    assert( m_pendingJobs.contains( job ) );

    const Collection collection = fjob->collections().first();
    const shared_ptr<Resource> resource = m_resources.value( m_pendingJobs.value( job ) );
    appendFeedCollection( collection, resource );
    emit q->feedAdded( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

// public interface implementation
FeedList::FeedList( QObject* parent )
    : QObject( parent ), d ( new FeedListPrivate( this ) )
{
}

FeedList::~FeedList()
{
    delete d;
}

QList<shared_ptr<Feed> > FeedList::feeds() const
{
    return d->m_feeds.values();
}

QList<shared_ptr<const Feed> > FeedList::constFeeds() const
{
    QList<shared_ptr<const Feed> > constFeeds;
    const QList<shared_ptr<Feed> > nonconstFeeds = d->m_feeds.values();
    Q_FOREACH( const shared_ptr<const Feed>& feed, nonconstFeeds )
        constFeeds.append( feed );

    return constFeeds;
}

QStringList FeedList::feedUrls() const {
    QStringList urls;
    Q_FOREACH( const shared_ptr<const Feed>& feed, d->m_feeds.values() ) {
        if ( !feed->isVirtual() ) {
            const shared_ptr<const NetFeed> netFeed = dynamic_pointer_cast<const NetFeed, const Feed>( feed );
            Q_ASSERT( netFeed );
            urls += netFeed->xmlUrl();
        }
    }
    return urls;
}

QList<shared_ptr<NetFeed> > FeedList::netFeeds() const
{
    QList<shared_ptr<NetFeed> > netFeeds;
    Q_FOREACH( const shared_ptr<Feed>& feed, d->m_feeds.values() ) {
        if ( !feed->isVirtual() ) {
            const shared_ptr<NetFeed> netFeed = dynamic_pointer_cast<NetFeed, Feed>( feed );
            Q_ASSERT( netFeed );
            netFeeds.append( netFeed );
        }
    }
    return netFeeds;
}

QList<shared_ptr<const NetFeed> > FeedList::constNetFeeds() const
{
    QList<shared_ptr<const NetFeed> > constNetFeeds;
    Q_FOREACH( const shared_ptr<const Feed>& feed, d->m_feeds.values() ) {
        if ( !feed->isVirtual() ) {
            const shared_ptr<const NetFeed> netFeed = dynamic_pointer_cast<const NetFeed, const Feed>( feed );
            Q_ASSERT( netFeed );
            constNetFeeds.append( netFeed );
        }
    }
    return constNetFeeds;
}

QList<shared_ptr<SearchFeed> > FeedList::searchFeeds() const
{
    QList<shared_ptr<SearchFeed> > searchFeeds;
    Q_FOREACH( const shared_ptr<Feed>& feed, d->m_feeds.values() ) {
        if ( feed->isVirtual() ) {
            const shared_ptr<SearchFeed> searchFeed = dynamic_pointer_cast<SearchFeed, Feed>( feed );
            Q_ASSERT( searchFeed );
            searchFeeds.append( searchFeed );
        }
    }
    return searchFeeds;
}

QList<shared_ptr<const SearchFeed> > FeedList::constSearchFeeds() const
{
    QList<shared_ptr<const SearchFeed> > constSearchFeeds;
    Q_FOREACH( const shared_ptr<const Feed>& feed, d->m_feeds.values() ) {
        if ( feed->isVirtual() ) {
            const shared_ptr<const SearchFeed> searchFeed = dynamic_pointer_cast<const SearchFeed, const Feed>( feed );
            Q_ASSERT( searchFeed );
            constSearchFeeds.append( searchFeed );
        }
    }
    return constSearchFeeds;
}

QList<Feed::Id> FeedList::feedIds() const
{
    return d->m_feeds.keys();
}

shared_ptr<Feed> FeedList::feedById( const Feed::Id& id ) const
{
    return d->m_feeds.value( id );
}

shared_ptr<const Feed> FeedList::constFeedById( const Feed::Id& id ) const
{
    return d->m_feeds.value( id );
}

QList<shared_ptr<Feed> > FeedList::feedByTitle( const QString& title ) const
{
    QList<shared_ptr<Feed> > feeds;

    Q_FOREACH( const shared_ptr<Feed>& feed, d->m_feeds.values() ) {
        if ( feed->title() == title )
            feeds.append( feed );
    }

    return feeds;
}

QList<shared_ptr<const Feed> > FeedList::constFeedByTitle( const QString& title ) const
{
    QList<shared_ptr<const Feed> > feeds;

    Q_FOREACH( const shared_ptr<const Feed>& feed, d->m_feeds.values() ) {
        if ( feed->title() == title )
            feeds.append( feed );
    }

    return feeds;
}

class KRss::RetrieveFeedListJobPrivate
{
    RetrieveFeedListJob* const q;
public:
    explicit RetrieveFeedListJobPrivate( RetrieveFeedListJob* qq );
    void loadDone( KJob* job );
    void doStart();

public:
    shared_ptr<FeedList> feedList;
    QHash<KJob*, Resource*> pendingJobs;
    QList<shared_ptr<Resource> > resources;
};

RetrieveFeedListJobPrivate::RetrieveFeedListJobPrivate( RetrieveFeedListJob* qq )
    : q( qq )
    , feedList( new FeedList )
{
}

RetrieveFeedListJob::RetrieveFeedListJob( QObject* parent ) : KJob( parent ), d( new RetrieveFeedListJobPrivate( this ) )
{
}

RetrieveFeedListJob::~RetrieveFeedListJob()
{
    delete d;
}

void RetrieveFeedListJob::setResources( const QList<shared_ptr<Resource> >& resources )
{
    d->resources = resources;
}

QList<shared_ptr<Resource> > RetrieveFeedListJob::resources() const
{
    return d->resources;
}

void RetrieveFeedListJobPrivate::loadDone( KJob* job_ )
{
    const RetrieveResourceCollectionsJob* const job = dynamic_cast<const RetrieveResourceCollectionsJob*>( job_ );
    assert( job );
    assert( pendingJobs.count() > 0 );
    Resource* const res = pendingJobs.take( job_ );

    if ( job->error() ) {
        q->setError( RetrieveFeedListJob::CouldNotRetrieveFeedList );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    Q_FOREACH( const shared_ptr<Resource>& resource, resources ) {
        if ( resource.get() == res ) {
            feedList->d->appendResourceCollections( job->collections(), resource );
            break;
        }
    }

    if ( pendingJobs.count() == 0 )
        q->emitResult();
}

shared_ptr<FeedList> RetrieveFeedListJob::feedList() const
{
    return d->feedList;
}

void RetrieveFeedListJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void RetrieveFeedListJobPrivate::doStart()
{
    if ( resources.isEmpty() ) {
        q->emitResult();
        return;
    }

    Q_FOREACH( const shared_ptr<Resource>& resource, resources ) {
        if ( !resource )
            continue;
        RetrieveResourceCollectionsJob* const job = resource->retrieveResourceCollectionsJob();
        q->connect( job, SIGNAL( result( KJob * ) ), q, SLOT( loadDone( KJob* ) ) );
        pendingJobs.insert( job, resource.get() );
        job->start();
    }
}

#include "feedlist.moc"
