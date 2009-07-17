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

#include "exportopmljob.h"
#include "rssresourcebase.h"
#include "rssresourcebasejobs.h"

#include <krss/tagprovider.h>

#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KLocale>
#include <KDebug>
#include <QtCore/QXmlStreamWriter>

using namespace KRssResource;
using boost::shared_ptr;

ExportOpmlJob::ExportOpmlJob( const KUrl& path, QObject *parent )
    : KJob( parent ), m_path( path )
{
}

void ExportOpmlJob::setResourceId( const QString& resourceId )
{
    Q_ASSERT( !resourceId.isEmpty() );
    m_resourceId = resourceId;
}

QString ExportOpmlJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case ExportOpmlJob::CouldNotRetrieveCollections:
            result = i18n( "Couldn't retrieve the collections from Akonadi.\n%1", errorText() );
            break;
        case ExportOpmlJob::CouldNotRetrieveTagProvider:
            result = i18n( "Couldn't retrieve the tag provider.\n%1", errorText() );
            break;
        case ExportOpmlJob::CouldNotExportFeeds:
            result = i18n( "Couldn't write the feeds out to the file.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

void ExportOpmlJob::start()
{
    Q_ASSERT( !m_resourceId.isEmpty() );
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void ExportOpmlJob::doStart()
{
    // we need all collections for this resource
    // invoke FeedCollectionRetrieveJob to let it fill the cache
    FeedCollectionRetrieveJob * const rjob = new FeedCollectionRetrieveJob( this );
    rjob->setResourceId( m_resourceId );
    connect( rjob, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsRetrieved( KJob* ) ) );
    rjob->start();
}

void ExportOpmlJob::slotCollectionsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotRetrieveCollections );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    KRss::TagProviderRetrieveJob * const tjob = new KRss::TagProviderRetrieveJob();
    connect( tjob, SIGNAL( result( KJob* ) ), this, SLOT( slotTagProviderRetrieved( KJob* ) ) );
    tjob->start();
}

void ExportOpmlJob::slotTagProviderRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotRetrieveTagProvider );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const KRss::TagProviderRetrieveJob * const rjob = qobject_cast<const KRss::TagProviderRetrieveJob*>( job );
    assert( rjob );

    const shared_ptr<const KRss::TagProvider> tagProvider = rjob->tagProvider();
    const QList<Akonadi::Collection> feeds = FeedCollectionsCache::feedCollections();

    QByteArray data;
    QXmlStreamWriter writer( &data );
    writer.setAutoFormatting( true );
    writer.writeStartDocument();
    writer.writeStartElement( "opml" );
    writer.writeAttribute( "version", "2.0" );
    writer.writeEmptyElement( "head" );
    writer.writeStartElement( "body" );
    Q_FOREACH( const KRss::FeedCollection& feed, feeds ) {
        writer.writeStartElement( "outline" );
        writer.writeAttribute( "remoteid", feed.remoteId() );
        writer.writeAttribute( "text", feed.title() );
        writer.writeAttribute( "title", feed.title() );
        writer.writeAttribute( "description", feed.description() );
        writer.writeAttribute( "htmlUrl", feed.htmlUrl() );
        writer.writeAttribute( "xmlUrl", feed.xmlUrl() );

        if ( !feed.tags().isEmpty() ) {
            const QList<KRss::TagId> tagIds = feed.tags();
            QStringList tagLabels;
            Q_FOREACH( const KRss::TagId& tagId, tagIds ) {
                tagLabels.append( tagProvider->tag( tagId ).label() );
            }
            writer.writeAttribute( "category", tagLabels.join( "," ) );
        }

        writer.writeEndElement();
    }
    writer.writeEndElement(); // body
    writer.writeEndElement(); // opml
    writer.writeEndDocument();

    KIO::TransferJob * const sjob = KIO::storedPut( data, m_path, -1, KIO::HideProgressInfo | KIO::Overwrite );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotPutFinished( KJob* ) ) );
    sjob->start();
}

void ExportOpmlJob::slotPutFinished( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
        setError( CouldNotExportFeeds );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}
