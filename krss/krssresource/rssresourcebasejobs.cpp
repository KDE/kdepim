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

#include "rssresourcebasejobs.h"
#include "rssbackendjobs.h"
#include "batchitemmodifyjob.h"
#include "rssitemsync.h"

#include <krss/item.h>
#include <krss/rssitem.h>

#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionCreateJob>
#include <Akonadi/CollectionModifyJob>
#include <Akonadi/CollectionDeleteJob>
#include <Akonadi/ItemSync>
#include <KGlobal>
#include <KLocale>
#include <KDebug>
#include <boost/bind.hpp>
#include <numeric>

//todo:
// - overloaded ctors
// - use FeedCollection only internally (Akonadi::Collection for public intefaces)

using namespace KRssResource;


// Cache that contains Akonadi collections owned by the resource.
// Normally, jobs declared in this file has direct access to it,
// but some jobs from the outside (namely, Import/ExportOpmlJob)
// which defined in separate files need to modify the cache as well.
// They have to use the functions defined in FeedCollectionsCache namespace.
struct FeedCollectionsCacheStruct
{
    FeedCollectionsCacheStruct() : m_cached( false ) {}
    KRss::FeedCollection m_rootCollection;
    QHash<KRss::Feed::Id, Akonadi::Collection> m_feeds;
    bool m_cached;
};

K_GLOBAL_STATIC( FeedCollectionsCacheStruct, s_feedCollectionsCache )

QList<Akonadi::Collection> KRssResource::FeedCollectionsCache::feedCollections()
{
    Q_ASSERT( s_feedCollectionsCache->m_cached );
    return s_feedCollectionsCache->m_feeds.values();
}

void KRssResource::FeedCollectionsCache::insertFeedCollection( const Akonadi::Collection& feed )
{
    Q_ASSERT( s_feedCollectionsCache->m_cached );
    s_feedCollectionsCache->m_feeds.insert( KRss::FeedCollection::feedIdFromAkonadi( feed.id() ), feed );
}

void KRssResource::FeedCollectionsCache::clear()
{
    s_feedCollectionsCache->m_rootCollection = Akonadi::Collection();
    s_feedCollectionsCache->m_feeds.clear();
    s_feedCollectionsCache->m_cached = false;
}

class FeedCollectionRetrieveJob::Private
{
public:
    explicit Private( const KRss::Feed::Id& id, FeedCollectionRetrieveJob *qq )
        : m_id( id ), q( qq ) {}

    void emitCachedResult();
    void slotCollectionsFetched( KJob *job );

    const KRss::Feed::Id m_id;
    QString m_resourceId;
    FeedCollectionRetrieveJob * const q;
};

void FeedCollectionRetrieveJob::Private::emitCachedResult()
{
    q->emitResult();
}

void FeedCollectionRetrieveJob::Private::slotCollectionsFetched( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionRetrieveJob::CouldNotFetchFeedCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const Akonadi::CollectionFetchJob * const fjob = qobject_cast<const Akonadi::CollectionFetchJob*>( job );
    Q_ASSERT( fjob );

    const QList<Akonadi::Collection> cols = fjob->collections();
    Q_FOREACH( const Akonadi::Collection& col, cols ) {
        if ( col.parent() == Akonadi::Collection::root().id() ) {
            s_feedCollectionsCache->m_rootCollection = col;
            kDebug() << "Root collection:" << col.id();
        }
        else {
            s_feedCollectionsCache->m_feeds.insert( KRss::FeedCollection::feedIdFromAkonadi( col.id() ), col );
        }
    }
    s_feedCollectionsCache->m_cached = true;

    const QList<Akonadi::Collection> feeds = s_feedCollectionsCache->m_feeds.values();
    Q_FOREACH( const KRss::FeedCollection& feed, feeds ) {
        kDebug() << "Cached feed:" << feed.id() << ", title:" << feed.title();
    }

    q->emitResult();
}

FeedCollectionRetrieveJob::FeedCollectionRetrieveJob( const KRss::Feed::Id& id, QObject *parent )
    : KJob( parent ), d( new Private( id, this ) )
{
}

FeedCollectionRetrieveJob::FeedCollectionRetrieveJob( QObject *parent )
    : KJob( parent ), d( new Private( KRss::Feed::Id(), this ) )
{
}

FeedCollectionRetrieveJob::~FeedCollectionRetrieveJob()
{
    delete d;
}

void FeedCollectionRetrieveJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    d->m_resourceId = resourceId;
}

KRss::FeedCollection FeedCollectionRetrieveJob::feedCollection() const
{
    return ( d->m_id == KRss::Feed::Id() ? s_feedCollectionsCache->m_rootCollection :
                             s_feedCollectionsCache->m_feeds.value( d->m_id ) );
}

QString FeedCollectionRetrieveJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCollectionRetrieveJob::CouldNotFetchFeedCollection:
            result = i18n( "Couldn't fetch collections from Akonadi.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedCollectionRetrieveJob::start()
{
    if ( s_feedCollectionsCache->m_cached ) {
        kDebug() << "Already cached";
        QMetaObject::invokeMethod( this, "emitCachedResult", Qt::QueuedConnection );
        return;
    }

    Q_ASSERT( !d->m_resourceId.isEmpty() );
    Akonadi::CollectionFetchJob * const job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(),
                                                                        Akonadi::CollectionFetchJob::Recursive );
    job->setResource( d->m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsFetched( KJob* ) ) );
    job->start();
}

class FeedCollectionCreateJob::Private
{
public:
    explicit Private( const QString& xmlUrl, FeedCollectionCreateJob *qq )
        : m_xmlUrl( xmlUrl ), m_backendJob( 0 ), q( qq ) {}

    void slotRootCollectionRetrieved( KJob *job );
    void slotBackendCollectionCreated( KJob *job );
    void slotAkonadiCollectionCreated( KJob *job );

    const QString m_xmlUrl;
    QString m_resourceId;
    KRssResource::FeedCreateJob *m_backendJob;
    FeedCollectionCreateJob * const q;
    KRss::FeedCollection m_rootCollection;
    KRss::FeedCollection m_collection;
};

void FeedCollectionCreateJob::Private::slotRootCollectionRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionCreateJob::CouldNotRetrieveRootCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    Q_ASSERT( rjob );
    m_rootCollection = rjob->feedCollection();
    kDebug() << "Found root collection:" << m_rootCollection.id();

    m_backendJob->setXmlUrl( m_xmlUrl );
    connect( m_backendJob, SIGNAL( result( KJob* ) ), q, SLOT( slotBackendCollectionCreated( KJob* ) ) );
    m_backendJob->start();
}

void FeedCollectionCreateJob::Private::slotBackendCollectionCreated( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionCreateJob::CouldNotCreateBackendCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const FeedCreateJob * const cjob = qobject_cast<const FeedCreateJob*>( job );
    Q_ASSERT( cjob );

    KRss::FeedCollection feed = cjob->feed();
    feed.setParent( m_rootCollection );
    kDebug() << "Added a new collection with remote id:" << feed.remoteId();
    Akonadi::CollectionCreateJob * const ajob = new Akonadi::CollectionCreateJob( feed );
    connect( ajob, SIGNAL( result( KJob* ) ), q, SLOT( slotAkonadiCollectionCreated( KJob* ) ) );
    ajob->start();
}

void FeedCollectionCreateJob::Private::slotAkonadiCollectionCreated( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionCreateJob::CouldNotCreateAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const Akonadi::CollectionCreateJob * const ajob = qobject_cast<const Akonadi::CollectionCreateJob*>( job );
    Q_ASSERT( ajob );

    m_collection = ajob->collection();
    FeedCollectionsCache::clear();
    q->emitResult();
}

FeedCollectionCreateJob::FeedCollectionCreateJob( const QString& xmlUrl, QObject *parent )
    : KJob( parent), d( new Private( xmlUrl, this ) )
{
}

FeedCollectionCreateJob::~FeedCollectionCreateJob()
{
    delete d;
}

void FeedCollectionCreateJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    d->m_resourceId = resourceId;
}

void FeedCollectionCreateJob::setBackendJob( KRssResource::FeedCreateJob *job )
{
    Q_ASSERT( job );
    d->m_backendJob = job;
}

KRss::FeedCollection FeedCollectionCreateJob::feedCollection() const
{
    return d->m_collection;
}

QString FeedCollectionCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCollectionCreateJob::CouldNotRetrieveRootCollection:
            result = i18n( "Couldn't retrieve the root collection from Akonadi.\n%1", errorText() );
            break;
        case FeedCollectionCreateJob::CouldNotCreateBackendCollection:
            result = i18n( "Couldn't create collection in the backend.\n%1", errorText() );
            break;
        case FeedCollectionCreateJob::CouldNotCreateAkonadiCollection:
            result = i18n( "Couldn't create collection in Akonadi.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedCollectionCreateJob::start()
{
    Q_ASSERT( !d->m_resourceId.isEmpty() );
    Q_ASSERT( d->m_backendJob );

    FeedCollectionRetrieveJob * const job = new FeedCollectionRetrieveJob( this );
    job->setResourceId( d->m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotRootCollectionRetrieved( KJob* ) ) );
    job->start();
}

class FeedCollectionModifyJob::Private
{
public:
    explicit Private( const KRss::Feed::Id& id, FeedCollectionModifyJob *qq )
        : m_id( id ), m_backendJob( 0 ), q( qq ) {}

    void slotAkonadiCollectionRetrieved( KJob *job );
    void slotAkonadiCollectionModified( KJob *job );

    const KRss::Feed::Id m_id;
    QString m_resourceId;
    KRssResource::FeedModifyJob *m_backendJob;
    FeedCollectionModifyJob * const q;
    boost::function1<void, KRss::FeedCollection&> m_modifier;
};

void FeedCollectionModifyJob::Private::slotAkonadiCollectionRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionModifyJob::CouldNotRetrieveAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    Q_ASSERT( rjob );
    KRss::FeedCollection collection = rjob->feedCollection();

    if ( m_modifier )
        m_modifier( collection );

    Akonadi::CollectionModifyJob * const ajob = new Akonadi::CollectionModifyJob( collection );
    connect( ajob, SIGNAL( result( KJob* ) ), q, SLOT( slotAkonadiCollectionModified( KJob* ) ) );
    ajob->start();
}

void FeedCollectionModifyJob::Private::slotAkonadiCollectionModified( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionModifyJob::CouldNotModifyAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    FeedCollectionsCache::clear();
    q->emitResult();
}

FeedCollectionModifyJob::FeedCollectionModifyJob( const KRss::Feed::Id& id, QObject *parent )
    : KJob( parent ), d( new Private( id, this ) )
{
}

FeedCollectionModifyJob::~FeedCollectionModifyJob()
{
    delete d;
}

void FeedCollectionModifyJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    d->m_resourceId = resourceId;
}

void FeedCollectionModifyJob::setBackendJob( KRssResource::FeedModifyJob *job )
{
    Q_ASSERT( job );
    d->m_backendJob = job;
}

void FeedCollectionModifyJob::setModifier( const boost::function1<void, KRss::FeedCollection&>& modifier )
{
    d->m_modifier = modifier;
}

QString FeedCollectionModifyJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCollectionModifyJob::CouldNotRetrieveAkonadiCollection:
            result = i18n( "Couldn't retrieve the collection from Akonadi.\n%1", errorText() );
            break;
        case FeedCollectionModifyJob::CouldNotModifyAkonadiCollection:
            result = i18n( "Couldn't modify collection in Akonadi.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedCollectionModifyJob::start()
{
    Q_ASSERT( !d->m_resourceId.isEmpty() );
    Q_ASSERT( d->m_backendJob );

    FeedCollectionRetrieveJob * const job = new FeedCollectionRetrieveJob( d->m_id, this );
    job->setResourceId( d->m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotAkonadiCollectionRetrieved( KJob* ) ) );
    job->start();
}

class FeedCollectionDeleteJob::Private
{
public:
    explicit Private( const KRss::Feed::Id& id, FeedCollectionDeleteJob *qq )
        : m_id( id ), m_backendJob( 0 ), q( qq ) {}

    void slotAkonadiCollectionRetrieved( KJob *job );
    void slotBackendCollectionDeleted( KJob *job );
    void slotAkonadiCollectionDeleted( KJob *job );

    const KRss::Feed::Id m_id;
    QString m_resourceId;
    KRssResource::FeedDeleteJob *m_backendJob;
    FeedCollectionDeleteJob * const q;
    KRss::FeedCollection m_collection;
};

void FeedCollectionDeleteJob::Private::slotAkonadiCollectionRetrieved( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionDeleteJob::CouldNotRetrieveAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    Q_ASSERT( rjob );
    m_collection = rjob->feedCollection();

    m_backendJob->setFeed( m_collection );
    connect( m_backendJob, SIGNAL( result( KJob* ) ), q, SLOT( slotBackendCollectionDeleted( KJob* ) ) );
    m_backendJob->start();
}

void FeedCollectionDeleteJob::Private::slotBackendCollectionDeleted( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionDeleteJob::CouldNotDeleteBackendCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    Akonadi::CollectionDeleteJob * const ajob = new Akonadi::CollectionDeleteJob( m_collection );
    connect( ajob, SIGNAL( result( KJob* ) ), q, SLOT( slotAkonadiCollectionDeleted( KJob* ) ) );
    ajob->start();
}

void FeedCollectionDeleteJob::Private::slotAkonadiCollectionDeleted( KJob *job )
{
    if ( job->error() ) {
        q->setError( FeedCollectionDeleteJob::CouldNotDeleteAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    FeedCollectionsCache::clear();
    q->emitResult();
}

FeedCollectionDeleteJob::FeedCollectionDeleteJob( const KRss::Feed::Id& id, QObject *parent )
    : KJob( parent ), d( new Private( id, this ) )
{
}

FeedCollectionDeleteJob::~FeedCollectionDeleteJob()
{
    delete d;
}

void FeedCollectionDeleteJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    d->m_resourceId = resourceId;
}

void FeedCollectionDeleteJob::setBackendJob( KRssResource::FeedDeleteJob *job )
{
    Q_ASSERT( job );
    d->m_backendJob = job;
}

QString FeedCollectionDeleteJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCollectionDeleteJob::CouldNotRetrieveAkonadiCollection:
            result = i18n( "Couldn't retrieve the collection from Akonadi.\n%1", errorText() );
            break;
        case FeedCollectionDeleteJob::CouldNotDeleteBackendCollection:
            result = i18n( "Couldn't delete collection in the backend.\n%1", errorText() );
            break;
        case FeedCollectionDeleteJob::CouldNotDeleteAkonadiCollection:
            result = i18n( "Couldn't delete collection in Akonadi.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedCollectionDeleteJob::start()
{
    Q_ASSERT( !d->m_resourceId.isEmpty() );
    Q_ASSERT( d->m_backendJob );

    FeedCollectionRetrieveJob * const job = new FeedCollectionRetrieveJob( d->m_id, this );
    job->setResourceId( d->m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotAkonadiCollectionRetrieved( KJob* ) ) );
    job->start();
}

class FeedCollectionFetchJob::Private
{
public:
    explicit Private( const KRss::FeedCollection& collection, FeedCollectionFetchJob * qq )
        : m_collection( collection ), m_id( KRss::Feed::Id() ), haveOnlyId( false ),
          m_synchronizeFlags( false ), m_backendJob( 0 ),
          m_percentages( QVector<uint>( 3, 0 ) ), q( qq ) {}

    explicit Private( const KRss::Feed::Id& id, FeedCollectionFetchJob * qq )
        : m_id( id ), haveOnlyId( true ), m_synchronizeFlags( false ), m_backendJob( 0 ),
        m_percentages( QVector<uint>( 3, 0 ) ), q( qq ) {}

    bool clearNewFlag( Akonadi::Item& item );
    void updatePercent();
    void doStart();
    void slotAkonadiCollectionRetrieved( KJob *job );
    void slotOldItemsMarked( KJob* );
    void slotFeedFetched( KJob *job );
    void slotItemsSynced( KJob *job );

    KRss::FeedCollection m_collection;
    const KRss::Feed::Id m_id;
    const bool haveOnlyId;
    QString m_resourceId;
    bool m_synchronizeFlags;
    KRssResource::FeedFetchJob *m_backendJob;
    QVector<uint> m_percentages;
    FeedCollectionFetchJob * const q;
};

bool FeedCollectionFetchJob::Private::clearNewFlag( Akonadi::Item& item )
{
    if ( item.hasFlag( KRss::RssItem::flagNew() ) ) {
        kDebug() << "Clearing 'New' on" << item.remoteId();
        item.clearFlag( KRss::RssItem::flagNew() );
        return true;
    }

    return false;
}

void FeedCollectionFetchJob::Private::updatePercent()
{
    const uint p = qBound( 0, qRound( std::accumulate( m_percentages.begin(), m_percentages.end(), 0u )
                              / static_cast<double>( m_percentages.size() ) ), 100 );
    q->setPercent( p );
}

void FeedCollectionFetchJob::Private::doStart()
{
    if ( haveOnlyId ) {
        FeedCollectionRetrieveJob * const job = new FeedCollectionRetrieveJob( m_id, q );
        job->setResourceId( m_resourceId );
        connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotAkonadiCollectionRetrieved( KJob* ) ) );
        job->start();
    }
    else {
        BatchItemModifyJob * const job = new BatchItemModifyJob();
        job->setFeed( m_collection );
        job->setModifier( boost::bind( &Private::clearNewFlag, this, _1 ) );
        connect( job, SIGNAL( result( KJob* ) ), q, SLOT( slotOldItemsMarked( KJob* ) ) );
        job->start();
    }
}

void FeedCollectionFetchJob::Private::slotAkonadiCollectionRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        q->setError( FeedCollectionFetchJob::CouldNotRetrieveAkonadiCollection );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    Q_ASSERT( rjob );
    m_collection = rjob->feedCollection();

    BatchItemModifyJob * const bjob = new BatchItemModifyJob();
    bjob->setFeed( m_collection );
    bjob->setModifier( boost::bind( &Private::clearNewFlag, this, _1 ) );
    connect( bjob, SIGNAL( result( KJob* ) ), q, SLOT( slotOldItemsMarked( KJob* ) ) );
    bjob->start();
}

void FeedCollectionFetchJob::Private::slotOldItemsMarked( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        q->setError( FeedCollectionFetchJob::CouldNotMarkOldItems );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    m_percentages.insert( 0, 100 );
    updatePercent();
    Q_ASSERT( m_backendJob );
    m_backendJob->setFeed( m_collection );
    connect( m_backendJob, SIGNAL( result( KJob* ) ), q, SLOT( slotFeedFetched( KJob* ) ) );
    m_backendJob->start();
}

void FeedCollectionFetchJob::Private::slotFeedFetched( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        q->setError( FeedCollectionFetchJob::CouldNotFetchFeed );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    m_percentages.insert( 1, 100 );
    updatePercent();
    const FeedFetchJob * const fjob = qobject_cast<const FeedFetchJob*>( job );
    Q_ASSERT( fjob );
    const QList<Akonadi::Item> items = fjob->items();

    RssItemSync * const syncer = new RssItemSync( m_collection );
    syncer->setIncrementalSyncItems( items, Akonadi::Item::List() );
    syncer->setSynchronizeFlags( m_synchronizeFlags );
    connect( syncer, SIGNAL( result( KJob* ) ), q, SLOT( slotItemsSynced( KJob* ) ) );
    syncer->start();
}

void FeedCollectionFetchJob::Private::slotItemsSynced( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        q->setError( FeedCollectionFetchJob::CouldNotSyncItems );
        q->setErrorText( job->errorString() );
        q->emitResult();
        return;
    }

    m_percentages.insert( 2, 100 );
    updatePercent();
    q->emitResult();
}

FeedCollectionFetchJob::FeedCollectionFetchJob( const KRss::FeedCollection& collection, QObject *parent )
    : KJob( parent ), d( new Private( collection, this ) )
{
}

FeedCollectionFetchJob::FeedCollectionFetchJob( const KRss::Feed::Id& id, QObject *parent )
    : KJob( parent ), d( new Private( id, this ) )
{
}

FeedCollectionFetchJob::~FeedCollectionFetchJob()
{
    delete d;
}

void FeedCollectionFetchJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    d->m_resourceId = resourceId;
}

void FeedCollectionFetchJob::setBackendJob( KRssResource::FeedFetchJob *job )
{
    Q_ASSERT( job );
    d->m_backendJob = job;
}

void FeedCollectionFetchJob::setSynchronizeFlags( bool synchronizeFlags )
{
    d->m_synchronizeFlags = synchronizeFlags;
}

bool FeedCollectionFetchJob::synchronizeFlags() const
{
    return d->m_synchronizeFlags;
}

KRss::FeedCollection FeedCollectionFetchJob::feedCollection() const
{
    return d->m_collection;
}

QString FeedCollectionFetchJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case FeedCollectionFetchJob::CouldNotMarkOldItems:
            result = i18n( "Couldn't reset the new flag on the old items.\n%1", errorText() );
            break;
        case FeedCollectionFetchJob::CouldNotFetchFeed:
            result = i18n( "Couldn't fetch the feed.\n%1", errorText() );
            break;
        case FeedCollectionFetchJob::CouldNotSyncItems:
            result = i18n( "Couldn't sync the fetched items with Akonadi.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void FeedCollectionFetchJob::start()
{
    Q_ASSERT( !d->m_resourceId.isEmpty() );
    Q_ASSERT( d->m_backendJob );
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

#include "rssresourcebasejobs.moc"
