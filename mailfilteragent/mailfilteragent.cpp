/*
    Copyright (c) 2011 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "mailfilteragent.h"

#include "dummykernel.h"
#include "filtermanager.h"
#include "mailfilteragentadaptor.h"

#include <akonadi/changerecorder.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/dbusconnectionpool.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/session.h>
#include <mailcommon/mailkernel.h>
#include <KLocalizedString>
#include <KMime/Message>

#include <QtCore/QTimer>

static bool isFilterableCollection( const Akonadi::Collection &collection )
{
  return MailCommon::Kernel::folderIsInbox( collection );

  //TODO: check got filter attribute here
}

MailFilterAgent::MailFilterAgent( const QString &id )
  : Akonadi::AgentBase( id )
{
  DummyKernel *kernel = new DummyKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes

  m_filterManager = new FilterManager( this );

  m_collectionMonitor = new Akonadi::Monitor( this );
  m_collectionMonitor->fetchCollection( true );
  m_collectionMonitor->setCollectionMonitored( Akonadi::Collection::root() );
  m_collectionMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  m_collectionMonitor->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
  m_collectionMonitor->setMimeTypeMonitored( KMime::Message::mimeType() );

  connect( m_collectionMonitor, SIGNAL( collectionAdded( Akonadi::Collection, Akonadi::Collection ) ),
           this, SLOT( mailCollectionAdded( Akonadi::Collection, Akonadi::Collection ) ) );
  connect( m_collectionMonitor, SIGNAL( collectionChanged( Akonadi::Collection ) ),
           this, SLOT( mailCollectionChanged( Akonadi::Collection ) ) );

  QTimer::singleShot( 0, this, SLOT( initializeCollections() ) );

  new MailFilterAgentAdaptor( this );

  Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/MailFilterAgent" ), this, QDBusConnection::ExportAdaptors );
  Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.MailFilterAgent" ) );

}

void MailFilterAgent::initializeCollections()
{
  m_filterManager->readConfig();

  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this );
  job->fetchScope().setContentMimeTypes( QStringList() << KMime::Message::mimeType() );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( initialCollectionFetchingDone( KJob* ) ) );
}

void MailFilterAgent::initialCollectionFetchingDone( KJob *job )
{
  if ( job->error() ) {
    qWarning() << job->errorString();
    return; //TODO: proper error handling
  }

  Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob*>( job );

  changeRecorder()->setMimeTypeMonitored( KMime::Message::mimeType() );
  changeRecorder()->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  changeRecorder()->itemFetchScope().setCacheOnly( true );
  if (m_filterManager->requiresFullMailBody()) {
    changeRecorder()->itemFetchScope().fetchFullPayload();
  } else {
    changeRecorder()->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Header, true );
  }
  changeRecorder()->fetchCollection( true );
  changeRecorder()->setChangeRecordingEnabled( false );

  foreach ( const Akonadi::Collection &collection, fetchJob->collections() ) {
    if ( isFilterableCollection( collection ) )
      changeRecorder()->setCollectionMonitored( collection, true );
  }
}

void MailFilterAgent::itemAdded( const Akonadi::Item &item, const Akonadi::Collection &collection )
{
  qDebug() << "MailFilterAgent: filter item" << item.id() << "in collection" << collection.name();

  m_filterManager->process( item, FilterManager::Inbound, true, collection.resource() );
}

void MailFilterAgent::mailCollectionAdded( const Akonadi::Collection &collection, const Akonadi::Collection& )
{
  if ( isFilterableCollection( collection ) )
  changeRecorder()->setCollectionMonitored( collection, true );
}

void MailFilterAgent::mailCollectionChanged( const Akonadi::Collection &collection )
{
  changeRecorder()->setCollectionMonitored( collection, isFilterableCollection( collection ) );
}

QString MailFilterAgent::createUniqueName( const QString &nameTemplate )
{
  return m_filterManager->createUniqueName( nameTemplate );
}

void MailFilterAgent::filter( const QVector<qlonglong> &itemIds, int filterSet )
{
  QList<Akonadi::Item> items;
  foreach ( qlonglong id, itemIds ) {
    items << Akonadi::Item( id );
  }

  m_filterManager->applyFilters( items, static_cast<FilterManager::FilterSet>(filterSet) );
}

void MailFilterAgent::filter( qlonglong item, int filterSet, const QString &resourceId )
{
  m_filterManager->filter( item, static_cast<FilterManager::FilterSet>( filterSet ), resourceId );
}

void MailFilterAgent::filter( qlonglong item, const QString &filterIdentifier )
{
  m_filterManager->filter( item, filterIdentifier );
}

void MailFilterAgent::reload()
{
  m_filterManager->readConfig();
}

AKONADI_AGENT_MAIN( MailFilterAgent )

#include "mailfilteragent.moc"
