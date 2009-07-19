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

#include "netresource.h"
#include "resource_p.h"
#include "exportopmljob.h"
#include "krssinterface.h"
#include "importitemsjob.h"
#include "importopmljob.h"
#include "feedcollection.h"
#include "retrieveresourcecollectionsjob.h"
#include "netfeedcreatejob.h"

#include <akonadi/collection.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/monitor.h>
#include <KDebug>

#include <QtCore/QString>

#include <cassert>

using namespace KRss;
using Akonadi::Collection;
using Akonadi::CollectionFetchJob;
using Akonadi::Monitor;
using boost::weak_ptr;

namespace KRss {

class NetResourcePrivate : public ResourcePrivate
{
public:
    NetResourcePrivate( const QString& resourceId, const QString& name, NetResource* qq )
        : ResourcePrivate( resourceId, name ), m_interface( 0 ), q( qq ) {}

    void slotCollectionAdded( const Akonadi::Collection& collection, const Akonadi::Collection& parent );
    void slotCollectionChanged( const Akonadi::Collection& collection );
    void slotCollectionRemoved( const Akonadi::Collection& collection );
    void slotCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                          const Akonadi::CollectionStatistics& statistics );
    void slotFetchStarted( qlonglong id );
    void slotFetchPercent( qlonglong id, uint percent );
    void slotFetchFinished( qlonglong id );
    void slotFetchFailed( qlonglong id, const QString& errorMessage );
    void slotFetchAborted( qlonglong id );
    void slotRootCollectionRetrieved( KJob* );

    org::kde::krss* m_interface;
    NetResource* const q;
};

} // namespace KRss

void NetResourcePrivate::slotCollectionAdded( const Akonadi::Collection& collection,
                                           const Akonadi::Collection& parent )
{
    Q_UNUSED( parent )
    const Feed::Id fid = FeedCollection::feedIdFromAkonadi( collection.id() );
    emit q->feedAdded( m_id, fid );
}

void NetResourcePrivate::slotCollectionChanged( const Akonadi::Collection& collection )
{
    q->triggerFeedChanged( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

void NetResourcePrivate::slotCollectionRemoved( const Akonadi::Collection& collection )
{
    q->triggerFeedRemoved( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

void NetResourcePrivate::slotCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                                          const Akonadi::CollectionStatistics& statistics )
{
    q->triggerStatisticsChanged( FeedCollection::feedIdFromAkonadi( id ), statistics );
}

void NetResourcePrivate::slotFetchStarted( qlonglong id )
{
    q->triggerFetchStarted( FeedCollection::feedIdFromAkonadi( id ) );
}

void NetResourcePrivate::slotFetchPercent( qlonglong id, uint percentage )
{
    q->triggerFetchPercent( FeedCollection::feedIdFromAkonadi( id ), percentage );
}

void NetResourcePrivate::slotFetchFinished( qlonglong id )
{
    q->triggerFetchFinished( FeedCollection::feedIdFromAkonadi( id ) );
}

void NetResourcePrivate::slotFetchFailed( qlonglong id, const QString& errorMessage )
{
    q->triggerFetchFailed( FeedCollection::feedIdFromAkonadi( id ), errorMessage );
}

void NetResourcePrivate::slotFetchAborted( qlonglong id )
{
    q->triggerFetchAborted( FeedCollection::feedIdFromAkonadi( id ) );
}

void NetResourcePrivate::slotRootCollectionRetrieved( KJob* j )
{
    const CollectionFetchJob* const job = qobject_cast<CollectionFetchJob*>( j );
    assert( job );

    const Collection::List collections = job->collections();
    if ( job->error() || collections.isEmpty() ) {
        kWarning() << "Could not retrieve root collection for resource " << m_id
                   << j->errorString();
        return;
    }

    const Collection root = collections.first();

    // create an Akonadi::Monitor
    Monitor* const monitor = new Monitor( q );
    monitor->fetchCollection( false );
    monitor->setCollectionMonitored( root, true );
    monitor->fetchCollectionStatistics( true );

    // monitor feeds
    QObject::connect( monitor, SIGNAL( collectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ),
                      q, SLOT( slotCollectionAdded( const Akonadi::Collection&, const Akonadi::Collection& ) ) );
    QObject::connect( monitor, SIGNAL( collectionChanged( const Akonadi::Collection& ) ),
                      q, SLOT( slotCollectionChanged( const Akonadi::Collection& )) );
    QObject::connect( monitor, SIGNAL( collectionRemoved( const Akonadi::Collection& ) ),
                      q, SLOT( slotCollectionRemoved( const Akonadi::Collection& )) );
    QObject::connect( monitor, SIGNAL( collectionStatisticsChanged(Akonadi::Collection::Id,
                                       Akonadi::CollectionStatistics) ),
                      q, SLOT( slotCollectionStatisticsChanged(Akonadi::Collection::Id,
                               Akonadi::CollectionStatistics)) );
}

NetResource::NetResource( const QString& resourceId, const QString& name, QObject* parent )
    : Resource( *new NetResourcePrivate( resourceId, name, this ), parent )
{
    Q_D( NetResource );
    d->m_interface = new org::kde::krss( "org.freedesktop.Akonadi.Agent." + d->m_id,
                                         "/KRss", QDBusConnection::sessionBus(), this );

    connect( d->m_interface, SIGNAL( fetchStarted( qlonglong ) ),
             this, SLOT( slotFetchStarted( qlonglong ) ) );
    connect( d->m_interface, SIGNAL( fetchPercent( qlonglong, uint ) ),
             this, SLOT( slotFetchPercent( qlonglong, uint ) ) );
    connect( d->m_interface, SIGNAL( fetchFinished( qlonglong ) ),
             this, SLOT( slotFetchFinished( qlonglong ) ) );
    connect( d->m_interface, SIGNAL( fetchFailed( qlonglong, const QString& ) ),
             this, SLOT( slotFetchFailed( qlonglong, const QString& ) ) );
    connect( d->m_interface, SIGNAL( fetchAborted( qlonglong ) ),
             this, SLOT( slotFetchAborted( qlonglong ) ) );
    connect( d->m_interface, SIGNAL( fetchQueueStarted() ),
             this, SIGNAL( fetchQueueStarted() ) );
    connect( d->m_interface, SIGNAL( fetchQueueFinished() ),
             this, SIGNAL( fetchQueueFinished() ) );

    CollectionFetchJob* const job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
    job->setResource( d->m_id );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRootCollectionRetrieved( KJob* ) ) );
    job->start();
}

ImportOpmlJob* NetResource::createImportOpmlJob( const KUrl& url ) const {
    Q_D( const NetResource );
    ImportOpmlJob* job = new ImportOpmlJob( d->m_interface->service() );
    job->setSourceUrl( url );
    return job;
}

ExportOpmlJob* NetResource::createExportOpmlJob( const KUrl& url ) const {
    Q_D( const NetResource );
    ExportOpmlJob* job = new ExportOpmlJob( d->m_interface->service() );
    job->setTargetUrl( url );
    return job;
}

ImportItemsJob* NetResource::createImportItemsJob( const QString& xmlUrl, const QString& sourceFile ) const {
    Q_D( const NetResource );
    ImportItemsJob* job = new ImportItemsJob( d->m_interface->service() );
    job->setFeedXmlUrl( xmlUrl );
    job->setSourceFile( sourceFile );
    return job;
}

RetrieveResourceCollectionsJob* NetResource::retrieveResourceCollectionsJob() const
{
    Q_D( const NetResource );
    return new RetrieveNetResourceCollectionsJob( d->m_id );
}

NetFeedCreateJob* NetResource::netFeedCreateJob( const QString& xmlUrl, const QString& subscriptionLabel )
{
    NetFeedCreateJob* job = new NetFeedCreateJob( xmlUrl, weak_ptr<NetResource>( shared_from_this() ) );
    job->setSubscriptionLabel( subscriptionLabel );
    return job;
}

void NetResource::addFeed( const QString& xmlUrl, const QString& subscriptionLabel ) const
{
    Q_D( const NetResource );
    d->m_interface->addFeed( xmlUrl, subscriptionLabel );
}

void NetResource::fetchFeed( const KRss::Feed::Id& feedId ) const
{
    Q_D( const NetResource );
    d->m_interface->fetchFeed( feedId );
}

void NetResource::abortFetch( const KRss::Feed::Id& feedId ) const
{
    Q_D( const NetResource );
    d->m_interface->abortFetch( feedId );
}

#include "netresource.moc"
