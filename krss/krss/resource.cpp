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
#include "exportopmljob.h"
#include "krssinterface.h"
#include "importitemsjob.h"
#include "importopmljob.h"
#include "feedcollection.h"

#include <akonadi/agentinstance.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/item.h>
#include <akonadi/monitor.h>
#include <akonadi/itemfetchscope.h>
#include <KDebug>

#include <QtCore/QString>

#include <cassert>

using Akonadi::Collection;
using Akonadi::CollectionFetchJob;
using Akonadi::AgentInstance;
using Akonadi::Monitor;

using namespace KRss;

namespace KRss {

class ResourcePrivate
{
public:
    ResourcePrivate( const AgentInstance& instance, Resource* qq )
        : m_instance( instance ), m_interface( 0 ), q( qq ) {}

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
    void slotRootCollectionFetchFinished( KJob* );

    AgentInstance m_instance;
    org::kde::krss *m_interface;
    Resource* const q;
};

} // namespace KRss

void ResourcePrivate::slotCollectionAdded( const Akonadi::Collection& collection,
                                           const Akonadi::Collection& parent )
{
    kDebug() << "Added. Col:" << collection.id() << ", parent:" << parent.id();
    emit q->feedAdded( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

void ResourcePrivate::slotCollectionChanged( const Akonadi::Collection& collection )
{
    kDebug() << "Changed. Col:" << collection.id();
    emit q->feedChanged( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

void ResourcePrivate::slotCollectionRemoved( const Akonadi::Collection& collection )
{
    kDebug() << "Removed. Col:" << collection.id();
    emit q->feedRemoved( FeedCollection::feedIdFromAkonadi( collection.id() ) );
}

void ResourcePrivate::slotCollectionStatisticsChanged( Akonadi::Collection::Id id,
                                                       const Akonadi::CollectionStatistics& statistics )
{
    emit q->statisticsChanged( FeedCollection::feedIdFromAkonadi( id ), statistics );
}

void ResourcePrivate::slotFetchStarted( qlonglong id )
{
    kDebug() << "Id:" << id;
    emit q->fetchStarted( FeedCollection::feedIdFromAkonadi( id ) );
}

void ResourcePrivate::slotFetchPercent( qlonglong id, uint percentage )
{
    kDebug() << "Id:" << id << " , percentage:" << percentage;
    emit q->fetchPercent( FeedCollection::feedIdFromAkonadi( id ), percentage );
}

void ResourcePrivate::slotFetchFinished( qlonglong id )
{
    kDebug() << "Id:" << id;
    emit q->fetchFinished( FeedCollection::feedIdFromAkonadi( id ) );
}

void ResourcePrivate::slotFetchFailed( qlonglong id, const QString& errorMessage )
{
    kDebug() << "Id:" << id << errorMessage;
    emit q->fetchFailed( FeedCollection::feedIdFromAkonadi( id ), errorMessage );
}

void ResourcePrivate::slotFetchAborted( qlonglong id )
{
    kDebug() << "Id:" << id;
    emit q->fetchAborted( FeedCollection::feedIdFromAkonadi( id ) );
}

void ResourcePrivate::slotRootCollectionFetchFinished( KJob* j )
{
    const CollectionFetchJob* const job = qobject_cast<CollectionFetchJob*>( j );
    assert( job );

    const Collection::List collections = job->collections();
    if ( job->error() || collections.isEmpty() ) {
        kWarning() << "Could not retrieve root collection for resource " << m_instance.identifier()
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


Resource::Resource( const AgentInstance &instance, QObject *parent )
    : QObject( parent ), d( new ResourcePrivate( instance, this ) )
{
    d->m_interface = new org::kde::krss( "org.freedesktop.Akonadi.Agent." + d->m_instance.identifier(), "/KRss",
                                         QDBusConnection::sessionBus(), this );

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

    CollectionFetchJob* job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::FirstLevel );
    job->setResource( d->m_instance.identifier() );
    connect( job, SIGNAL(finished(KJob*)), this, SLOT(slotRootCollectionFetchFinished(KJob*)) );
}

Resource::~Resource()
{
    delete d;
}

ImportOpmlJob* Resource::createImportOpmlJob( const KUrl& url ) const {
    ImportOpmlJob* job = new ImportOpmlJob( d->m_interface->service() );
    job->setSourceUrl( url );
    return job;
}

ExportOpmlJob* Resource::createExportOpmlJob( const KUrl& url ) const {
    ExportOpmlJob* job = new ExportOpmlJob( d->m_interface->service() );
    job->setTargetUrl( url );
    return job;
}

ImportItemsJob* Resource::createImportItemsJob( const QString& xmlUrl, const QString& sourceFile ) const {
    ImportItemsJob* job = new ImportItemsJob( d->m_interface->service() );
    job->setFeedXmlUrl( xmlUrl );
    job->setSourceFile( sourceFile );
    return job;
}

bool Resource::isValid() const
{
    return d->m_instance.isValid();
}

void Resource::addFeed( const QString &xmlUrl, const QString &subscriptionLabel ) const
{
    d->m_interface->addFeed( xmlUrl, subscriptionLabel );
}

void Resource::fetchFeed( const KRss::Feed::Id& feedId ) const
{
    d->m_interface->fetchFeed( feedId );
}

void Resource::abortFetch( const KRss::Feed::Id& feedId ) const
{
    d->m_interface->abortFetch( feedId );
}

#include "resource.moc"
