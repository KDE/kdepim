/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>
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

#include "importopmljob.h"
#include "rssresourcebase.h"
#include "rssresourcebasejobs.h"
#include "rssbackendjobs.h"
#include "util.h"

#include <krss/tagjobs.h>
#include <krss/tagprovider.h>

#include <akonadi/collectioncreatejob.h>

#include <KUrl>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KLocale>
#include <KDebug>
#include <QtCore/QXmlStreamReader>

#include <algorithm>
#include <cassert>

using namespace KRssResource;
using boost::shared_ptr;

ImportOpmlJob::ImportOpmlJob( const KUrl& path, QObject *parent )
    : KJob( parent ), m_backendJob( 0 ), m_path( path ), m_pendingJobs( 0 )
{
}

void ImportOpmlJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    m_resourceId = resourceId;
}

void ImportOpmlJob::setBackendJob( KRssResource::FeedsImportJob *job )
{
    Q_ASSERT( job );
    m_backendJob = job;
}

void ImportOpmlJob::setDefaultTag( const QString& defaultTag )
{
    m_defaultTag = defaultTag;
}

QList<Akonadi::Collection> ImportOpmlJob::importedFeeds() const
{
    return m_importedFeeds;
}

QString ImportOpmlJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case ImportOpmlJob::CouldNotReadOpml:
            result = i18n( "Could not read in the OPML document.\n%1", errorText() );
            break;
        case ImportOpmlJob::CouldNotParseOpml:
            result = i18n( "Could not parse the OPML document.\n%1", errorText() );
            break;
        case ImportOpmlJob::CouldNotImportTags:
            result = i18n( "Could not import the tags from the OPML document.\n%1", errorText() );
            break;
        case ImportOpmlJob::CouldNotImportFeeds:
            result = i18n( "Could not import the feeds from the OPML document.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void ImportOpmlJob::start()
{
    Q_ASSERT( !m_resourceId.isEmpty() );
    Q_ASSERT( m_backendJob );
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ImportOpmlJob::doStart()
{
    KIO::StoredTransferJob * const job = KIO::storedGet( m_path, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotOpmlRead( KJob* ) ) );
    job->start();
}

void ImportOpmlJob::slotOpmlRead( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotReadOpml );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    // find the first start element, it should be 'Opml'
    const KIO::StoredTransferJob * const tjob = qobject_cast<const KIO::StoredTransferJob*>( job );
    Q_ASSERT( tjob );

    QXmlStreamReader reader( tjob->data() );
    while ( !reader.atEnd() ) {
        reader.readNext();
        if ( reader.isStartElement() )
            break;
    }

    if ( reader.name().toString().toLower() == "opml" ) {
        kDebug() << "OPML version" << reader.attributes().value( "version" ).toString();
        m_opmlReader.readOpml( reader );
    }
    else {
        setError( CouldNotParseOpml );
        setErrorText( i18n( "The file is not a valid OPML document" ) );
        kWarning() << errorText();
        emitResult();
        return;
    }

    if ( reader.hasError() ) {
        setError( CouldNotParseOpml );
        setErrorText( reader.errorString() );
        kWarning() << errorText();
        emitResult();
        return;
    }

    FeedCollectionRetrieveJob * const rjob = new FeedCollectionRetrieveJob( this );
    rjob->setResourceId( m_resourceId );
    connect( rjob, SIGNAL( result( KJob* ) ), this, SLOT( slotRootCollectionRetrieved( KJob* ) ) );
    rjob->start();
}

void ImportOpmlJob::slotRootCollectionRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotRetrieveRootCollection );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    // save the root collection, will be used later
    const FeedCollectionRetrieveJob * const rjob = qobject_cast<const FeedCollectionRetrieveJob*>( job );
    assert( rjob );
    m_rootCollection = rjob->feedCollection();

    KRss::TagProviderRetrieveJob * const tjob = new KRss::TagProviderRetrieveJob();
    connect( tjob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void ImportOpmlJob::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotImportTags );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const KRss::TagProviderRetrieveJob * const rjob = qobject_cast<const KRss::TagProviderRetrieveJob*>( job );
    assert( rjob );

    TagsCreateJob * const tjob = new TagsCreateJob();
    tjob->setTagProvider( rjob->tagProvider() );

    tjob->setTagLabels( m_defaultTag.isEmpty() ? m_opmlReader.tags() : ( m_opmlReader.tags() << m_defaultTag ) );
    connect( tjob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagsCreated( KJob* ) ) );
    tjob->start();
}

void ImportOpmlJob::slotTagsCreated( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotImportTags );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const TagsCreateJob * const tjob = qobject_cast<const TagsCreateJob*>( job );
    assert( job );
    const QList<KRss::Tag> tags = tjob->tags();

    int currentFeed = 0;
    const QList<ParsedFeed> parsedFeeds = m_opmlReader.feeds();
    QList<Akonadi::Collection> feedsToImport;
    Q_FOREACH( const ParsedFeed& parsedFeed, parsedFeeds ) {
        KRss::FeedCollection feed = parsedFeed.toAkonadiCollection();
        feed.setParent( m_rootCollection );
        const QList<int> tagIndexes = m_opmlReader.tagsForFeeds().value( currentFeed );
        kDebug() << "Current feed:" << feed.title();
        Q_FOREACH( int tagIndex, tagIndexes ) {
            kDebug() << "Adding tag:" << tags.at( tagIndex ).label() << ", id:" << tags.at( tagIndex ).id();
            feed.addTag( tags.at( tagIndex ).id() );
        }
        if ( !m_defaultTag.isEmpty() )
            feed.addTag( tags.last().id() ); // the last one is the default tag
        ++currentFeed;
        feedsToImport.append( feed );
    }

    m_backendJob->setFeeds( feedsToImport );
    connect( m_backendJob, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedsImportedBackend( KJob* ) ) );
    m_backendJob->start();
}

void ImportOpmlJob::slotFeedsImportedBackend( KJob *job )
{
    kDebug();
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotImportFeeds );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const FeedsImportJob * const ijob = qobject_cast<const FeedsImportJob*>( job );
    assert( ijob );

    const QList<Akonadi::Collection> feeds = ijob->feeds();
    Q_FOREACH( KRss::FeedCollection feed, feeds ) {
        feed.setName( generateCollectionName( feed ) );
        Akonadi::CollectionCreateJob * const cjob = new Akonadi::CollectionCreateJob( feed );
        kDebug() << "created job for" << feed.title() << feed.name() << feed.description();
        connect( cjob, SIGNAL( result( KJob* ) ), this, SLOT( slotFeedImportedAkonadi( KJob* ) ) );
        cjob->start();
        ++m_pendingJobs;
        kDebug() << m_pendingJobs;
    }

    if ( m_pendingJobs == 0 )
        emitResult();
}

void ImportOpmlJob::slotFeedImportedAkonadi( KJob *job )
{
    kDebug() << m_pendingJobs;
    assert( m_pendingJobs > 0 );
    --m_pendingJobs;

    if ( job->error() && !error() ) {
        setError( CouldNotImportFeeds );
        setErrorText( job->errorString() );
    }
    else {
        const Akonadi::CollectionCreateJob * const cjob = qobject_cast<Akonadi::CollectionCreateJob*>( job );
        assert( cjob );
        m_importedFeeds.append( cjob->collection() );
    }

    if ( m_pendingJobs > 0 )
        return;

    FeedCollectionsCache::clear();
    emitResult();
}

TagsCreateJob::TagsCreateJob( QObject *parent )
    : KJob( parent ), m_pendingJobs( 0 )
{
}

void TagsCreateJob::setTagProvider( const shared_ptr<const KRss::TagProvider>& tagProvider )
{
    m_tagProvider = tagProvider;
}

void TagsCreateJob::setTagLabels( const QList<QString>& tagLabels )
{
    m_tagLabels = tagLabels;
}

QList<KRss::Tag> TagsCreateJob::tags() const
{
    return m_tags.toList();
}

QString TagsCreateJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case TagsCreateJob::CouldNotCreateTags:
            result = i18n( "Could not create the tags.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void TagsCreateJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void TagsCreateJob::doStart()
{
    if ( !m_tagProvider || m_tagLabels.isEmpty() ) {
        emitResult();
        return;
    }

    m_tags.fill( KRss::Tag(), m_tagLabels.count() );
    Q_FOREACH( const QString& label, m_tagLabels ) {
        KRss::Tag tag;
        tag.setLabel( label );
        KRss::TagCreateJob* job = m_tagProvider->tagCreateJob();
        connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotTagCreated( KJob* ) ) );
        job->setTag( tag );
        ++m_pendingJobs;
        job->start();
    }
}

void TagsCreateJob::slotTagCreated( KJob* job )
{
    assert( m_pendingJobs > 0 );
    --m_pendingJobs;

    if ( job->error() && !error() ) {
        setError( CouldNotCreateTags );
        setErrorText( job->errorString() );
    }

    const KRss::TagCreateJob * const tjob = qobject_cast<const KRss::TagCreateJob*>( job );
    assert( tjob );
    m_tags.replace( m_tagLabels.indexOf( tjob->tag().label() ), tjob->tag() );

    if ( m_pendingJobs > 0 )
        return;

    emitResult();
}

#include "importopmljob.moc"
