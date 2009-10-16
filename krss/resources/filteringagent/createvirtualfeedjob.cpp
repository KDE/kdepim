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

#include "createvirtualfeedjob.h"
#include "krss/virtualfeedpropertiesattribute.h"

#include <Akonadi/SearchCreateJob>
#include <Akonadi/CollectionFetchJob>
#include <Akonadi/CollectionModifyJob>
#include <KRandom>

using Akonadi::Collection;
using Akonadi::SearchCreateJob;
using Akonadi::CollectionFetchJob;
using Akonadi::CollectionModifyJob;
using namespace KRss;

CreateVirtualFeedJob::CreateVirtualFeedJob( const QString& name, const QString& agentId, QObject* parent )
    : KJob( parent ), m_name( name ), m_agentId( agentId )
{
    m_virtualFeed.attribute<VirtualFeedPropertiesAttribute>( Collection::AddIfMissing )->setTitle( name );
    m_virtualFeed.setName( QLatin1String( "RSS-filtering-feed-" ) + m_name );
    m_virtualFeed.setRemoteId( QLatin1String( "RSS-filtering-feed-" ) + KRandom::randomString( 10 ) );
}

void CreateVirtualFeedJob::start()
{
    QMetaObject::invokeMethod( this, "doStart", Qt::QueuedConnection );
}

QString CreateVirtualFeedJob::errorString() const
{
    return QString();
}

void CreateVirtualFeedJob::doStart()
{
    SearchCreateJob* const sjob = new SearchCreateJob( m_virtualFeed.name(),
                                                       m_virtualFeed.remoteId(),
                                                       this );
    connect( sjob, SIGNAL( result( KJob* ) ), this, SLOT( slotVirtualFeedCreated( KJob* ) ) );
    sjob->start();
}

void CreateVirtualFeedJob::slotVirtualFeedCreated( KJob* job )
{
    if ( job->error() ) {
        setError( CouldNotCreateVirtualFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    CollectionFetchJob* const fjob = new CollectionFetchJob( Collection( 1 ),
                                                             CollectionFetchJob::FirstLevel, this );
    connect( fjob, SIGNAL( result( KJob* ) ), this, SLOT( slotVirtualFeedRetrieved( KJob* ) ) );
    fjob->start();
}

void CreateVirtualFeedJob::slotVirtualFeedRetrieved( KJob* job )
{
    if ( job->error() ) {
        setError( CouldNotRetrieveVirtualFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    const CollectionFetchJob* const fjob = qobject_cast<CollectionFetchJob*>( job );
    Q_ASSERT( fjob );
    const QList<Collection> cols = fjob->collections();
    Q_FOREACH( const Collection& col, cols ) {
        if ( col.remoteId() == m_virtualFeed.remoteId() ) {
            kDebug() << "Found the just created virtual feed at id:" << col.id();
            m_virtualFeed.setId( col.id() );
            CollectionModifyJob* const mjob = new CollectionModifyJob( m_virtualFeed );
            connect( mjob, SIGNAL( result( KJob* ) ), this, SLOT( slotVirtualFeedModified( KJob* ) ) );
            mjob->start();
            return;
        }
    }

    kWarning() << "Could not find the just created filtering feed in Akonadi";
    setError( CouldNotRetrieveVirtualFeed );
    emitResult();
}

void CreateVirtualFeedJob::slotVirtualFeedModified( KJob *job )
{
    if ( job->error() ) {
        setError( CouldNotModifyVirtualFeed );
        setErrorText( job->errorString() );
        emitResult();
        return;
    }

    emitResult();
}
