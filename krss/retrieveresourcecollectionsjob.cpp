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

#include "retrieveresourcecollectionsjob.h"

#include <KLocale>
#include <Akonadi/Collection>
#include <Akonadi/CollectionFetchJob>

using namespace KRss;
using Akonadi::Collection;
using Akonadi::CollectionFetchJob;

RetrieveNetResourceCollectionsJob::RetrieveNetResourceCollectionsJob( const QString& resourceId,
                                                                      QObject* parent )
    : RetrieveResourceCollectionsJob( parent ), m_resourceId( resourceId )
{
    assert( !resourceId.isEmpty() );
}

QString RetrieveNetResourceCollectionsJob::errorString() const
{
    QString result;
    switch ( error() ) {
        case KJob::NoError:
            result = i18n( "No error." );
            break;
        case RetrieveResourceCollectionsJob::CouldNotRetrieveCollections:
            result = i18n( "Could not retrieve collections from the resource.\n%1", errorText() );
            break;
        default:
            result = i18n( "Unknown result code." );
            break;
    }
    return result;
}

QList<Collection> RetrieveNetResourceCollectionsJob::collections() const
{
    return m_collections;
}

void RetrieveNetResourceCollectionsJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

void RetrieveNetResourceCollectionsJob::doStart()
{
    CollectionFetchJob* const job = new CollectionFetchJob( Collection::root(), CollectionFetchJob::Recursive );
    job->includeUnsubscribed( true );
    job->setResource( m_resourceId );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotCollectionsRetrieved( KJob* ) ) );
    job->start();
}

void RetrieveNetResourceCollectionsJob::slotCollectionsRetrieved( KJob* job )
{
    const CollectionFetchJob* const cjob = qobject_cast<const CollectionFetchJob*>( job );
    assert( cjob );

    if ( cjob->error() ) {
        setError( RetrieveResourceCollectionsJob::CouldNotRetrieveCollections );
        setErrorText( cjob->errorString() );
        emitResult();
        return;
    }

    const QList<Collection> cols = cjob->collections();
    Q_FOREACH( const Collection& col, cols ) {
        if ( col.parent() != Collection::root().id() )
            m_collections.append( col );
    }

    emitResult();
}

#include "retrieveresourcecollectionsjob.moc"
