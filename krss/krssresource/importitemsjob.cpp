/*
    Copyright (C) 2009    Frank Osterfeld <osterfeld@kde.org>

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

#include "importitemsjob.h"
#include "rssitemsync.h"
#include "rssresourcebasejobs.h"
#include "krss/rssitem.h"

#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>
#include <Akonadi/ItemFetchJob>

#include <KLocalizedString>
#include <QFile>
#include <QString>

#include <algorithm>

using KRss::RssItem;
using namespace KRssResource;

ImportItemsJob::ImportItemsJob( const QString& xmlUrl, QObject *parent )
    : KJob( parent ), m_xmlUrl( xmlUrl ), m_flagsSynchronizable( false ), m_reader( 0 ), m_pendingCreateJobs( 0 ) {

}

ImportItemsJob::~ImportItemsJob() {
    delete m_reader;
}

void ImportItemsJob::cleanupAndEmitResult() {
    delete m_reader;
    m_reader = 0;
    m_file.close();
    emitResult();
}

void ImportItemsJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    m_resourceId = resourceId;
}

void ImportItemsJob::setFlagsSynchronizable( bool flagsSynchronizable )
{
    m_flagsSynchronizable = flagsSynchronizable;
}

void ImportItemsJob::setSourceFile( const QString& fn ) {
    m_fileName = fn;
}

void ImportItemsJob::start() {
    Q_ASSERT( !m_resourceId.isEmpty() );
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ImportItemsJob::doStart() {
    FeedCollectionRetrieveJob *job = new FeedCollectionRetrieveJob( this );
    job->setResourceId( m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionRetrieved( KJob* ) ) );
    job->start();
}

void ImportItemsJob::slotCollectionRetrieved( KJob *job ) {
    if ( job->error() ) {
        setError( CouldNotRetrieveCollection );
        setErrorText( i18n( "Could not retrieve the collections from Akonadi.\n%1", job->errorString() ) );
        cleanupAndEmitResult();
        return;
    }

    const FeedCollectionRetrieveJob * const fjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    assert( fjob );

    const QList<Akonadi::Collection> feeds = FeedCollectionsCache::feedCollections();
    Q_FOREACH( const KRss::FeedCollection& feed, feeds ) {
        if ( feed.xmlUrl() == m_xmlUrl ) {
            m_collection = feed;
            break;
        }
    }

    if ( !m_collection.isValid() ) {
        setError( CouldNotRetrieveCollection );
        setErrorText( i18n( "No such collection. XML Url:%1", m_xmlUrl ) );
        cleanupAndEmitResult();
        return;
    }

    Akonadi::ItemFetchJob* itemJob = new Akonadi::ItemFetchJob( m_collection, this );
    //TODO set fetch scope
    connect( itemJob, SIGNAL(result(KJob*)), this, SLOT(slotItemsFetched(KJob*)) );
}

static bool lessByRemoteId( const Akonadi::Item& lhs, const Akonadi::Item& rhs ) {
    return lhs.remoteId() < rhs.remoteId();
}

static bool equalByRemoteId( const Akonadi::Item& lhs, const Akonadi::Item& rhs ) {
    return lhs.remoteId() == rhs.remoteId();
}

void ImportItemsJob::slotItemsFetched( KJob* j ) {
    const Akonadi::ItemFetchJob* const job = qobject_cast<const Akonadi::ItemFetchJob*>( j );
    assert( job );
    if ( job->error() ) {
        setError( ItemSyncFailed );
        setErrorText( i18n( "No not retrieve existing icons for syncing: %1", job->errorString() ) );
        cleanupAndEmitResult();
        return;
    }

    m_existingItems = job->items();
    std::sort( m_existingItems.begin(), m_existingItems.end(), lessByRemoteId );
    startImport();
}

void ImportItemsJob::startImport() {
    m_file.setFileName( m_fileName );
    if ( !m_file.open( QIODevice::ReadOnly ) ) {
        setError( CouldNotOpenFile );
        setErrorText( i18n("Could not open file %1 for reading.", m_fileName ) );
        cleanupAndEmitResult();
        return;
    }
    m_reader = new ItemImportReader( &m_file );
    readBatch();
}

void ImportItemsJob::readBatch() {
    int num = 500;

    Akonadi::Item::List items;

    while ( num > 0 && m_reader->hasNext() ) {
        const Akonadi::Item item = m_reader->nextItem();
        if ( !item.remoteId().isEmpty() )
            items.append( item );
        --num;
    }

    if ( items.isEmpty() ) {
        cleanupAndEmitResult();
        return;
    }

    syncItems( items );
}

void ImportItemsJob::syncItems( const Akonadi::Item::List& items_ ) {
    Akonadi::Item::List items = items_;
    std::sort( items.begin(), items.end(), lessByRemoteId );

    //split into existing items (must be merged) and new items (will be just added)
    Akonadi::Item::List toMerge;
    Akonadi::Item::List toAdd;

    Akonadi::Item::List::const_iterator itRead = items.constBegin();

    if ( m_existingItems.isEmpty() )
        toAdd = items;
    else {
        Akonadi::Item::List::const_iterator itExisting = m_existingItems.constBegin();
        bool foundLast = false;
        for ( ; itRead != items.constEnd(); ++itRead ) {
            if ( foundLast ) {
                toAdd.push_back( *itRead );
                continue;
            }
            itExisting = std::lower_bound( itExisting, m_existingItems.constEnd(), *itRead, lessByRemoteId );
            foundLast = itExisting == items.constEnd();
            if ( foundLast ) {
                toAdd.push_back( *itRead );
                continue;
            }

            if ( equalByRemoteId( *itExisting, *itRead ) )
                toMerge.push_back( *itRead );
            else
                toAdd.push_back( *itRead );
        }
    }

    //TODO: merge flags etc. for existing items

    if ( toAdd.isEmpty() ) {
        QMetaObject::invokeMethod( this, "readBatch", Qt::QueuedConnection );
        return;
    }

    Q_FOREACH( const Akonadi::Item& item, toAdd ) {
        Akonadi::ItemCreateJob* const job = new Akonadi::ItemCreateJob( item, m_collection );
        ++m_pendingCreateJobs;
        QObject::connect(job, SIGNAL(result(KJob*)), this, SLOT(syncDone(KJob*)) );
        job->start();
    }
}

void ImportItemsJob::syncDone( KJob* job ) {
    if ( job->error() ) {
        setError( ItemSyncFailed );
        setErrorText( job->errorText() );
        cleanupAndEmitResult();
        return;
    }
    assert( m_pendingCreateJobs > 0 );
    --m_pendingCreateJobs;
    if ( m_pendingCreateJobs == 0 )
       readBatch();
}
