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

#include "dbusoperators.h"
#include "dummykernel.h"
#include "filterlogdialog.h"
#include "filtermanager.h"
#include "mailfilteragentadaptor.h"

#include <akonadi/changerecorder.h>
#include <akonadi/collectionfetchjob.h>
#include <akonadi/collectionfetchscope.h>
#include <akonadi/dbusconnectionpool.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/kmime/messageparts.h>
#include <akonadi/kmime/messagestatus.h>
#include <akonadi/session.h>
#include <mailcommon/mailkernel.h>
#include <KLocalizedString>
#include <KMime/Message>
#include <KNotification>
#include <KWindowSystem>

#include <QtCore/QVector>
#include <QtCore/QTimer>

static bool isFilterableCollection( const Akonadi::Collection &collection )
{
  return MailCommon::Kernel::folderIsInbox( collection );

  //TODO: check got filter attribute here
}

MailFilterAgent::MailFilterAgent( const QString &id )
  : Akonadi::AgentBase( id ),
    m_filterLogDialog( 0 )
{
  DummyKernel *kernel = new DummyKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  m_filterManager = new FilterManager( this );

  m_collectionMonitor = new Akonadi::Monitor( this );
  m_collectionMonitor->fetchCollection( true );
  m_collectionMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  m_collectionMonitor->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
  m_collectionMonitor->setMimeTypeMonitored( KMime::Message::mimeType() );

  connect( m_collectionMonitor, SIGNAL(collectionAdded(Akonadi::Collection,Akonadi::Collection)),
           this, SLOT(mailCollectionAdded(Akonadi::Collection,Akonadi::Collection)) );
  connect( m_collectionMonitor, SIGNAL(collectionChanged(Akonadi::Collection)),
           this, SLOT(mailCollectionChanged(Akonadi::Collection)) );

  connect( m_collectionMonitor, SIGNAL(collectionRemoved(Akonadi::Collection)),
           this, SLOT(mailCollectionRemoved(Akonadi::Collection)) );

  QTimer::singleShot( 0, this, SLOT(initializeCollections()) );

  qDBusRegisterMetaType<QVector<qlonglong> >();

  new MailFilterAgentAdaptor( this );

  Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/MailFilterAgent" ), this, QDBusConnection::ExportAdaptors );
  Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.MailFilterAgent" ) );
  //Enabled or not filterlogdialog
  KSharedConfig::Ptr config = KGlobal::config();
  if ( config->hasGroup( "FilterLog" ) ) {
    KConfigGroup group( config, "FilterLog" );
    if ( group.hasKey( "Enabled" ) ) {
      if ( group.readEntry( "Enabled", false ) ) {
          m_filterLogDialog = new FilterLogDialog( 0 );
          const QPixmap pixmap = KIcon( "view-filter" ).pixmap( KIconLoader::SizeSmall, KIconLoader::SizeSmall );
          KNotification *notify = new KNotification( "mailfilterlogenabled" );
          notify->setComponentData( componentData() );
          notify->setPixmap( pixmap );
          notify->setText( i18nc("Notification when the filter log was enabled", "Mail Filter Log Enabled" ) );
          notify->sendEvent();
      }
    }
  }
}

void MailFilterAgent::initializeCollections()
{
  m_filterManager->readConfig();

  Akonadi::CollectionFetchJob *job = new Akonadi::CollectionFetchJob( Akonadi::Collection::root(), Akonadi::CollectionFetchJob::Recursive, this );
  job->fetchScope().setContentMimeTypes( QStringList() << KMime::Message::mimeType() );
  connect( job, SIGNAL(result(KJob*)), this, SLOT(initialCollectionFetchingDone(KJob*)) );
}

void MailFilterAgent::initialCollectionFetchingDone( KJob *job )
{
  if ( job->error() ) {
    qWarning() << job->errorString();
    return; //TODO: proper error handling
  }

  Akonadi::CollectionFetchJob *fetchJob = qobject_cast<Akonadi::CollectionFetchJob*>( job );

  changeRecorder()->itemFetchScope().setAncestorRetrieval( Akonadi::ItemFetchScope::Parent );
  changeRecorder()->itemFetchScope().setCacheOnly(false);
  mRequestedPart = m_filterManager->requiredPart();
  if (mRequestedPart == MailCommon::SearchRule::CompleteMessage) {
    changeRecorder()->itemFetchScope().fetchFullPayload();
  } else if (mRequestedPart == MailCommon::SearchRule::Header) {
    changeRecorder()->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Header, true );
  } else {
    changeRecorder()->itemFetchScope().fetchPayloadPart( Akonadi::MessagePart::Envelope, true );
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
  /* The monitor mimetype filter would override the collection filter, therefor we have to check
   * for the mimetype of the item here.
   */
  if ( item.mimeType() != KMime::Message::mimeType() )
    return;

  /*
   * happens when item no longer exists etc, and queue compression didn't happen yet
   */
  if ( !item.hasPayload() )
    return;

  Akonadi::MessageStatus status;
  status.setStatusFromFlags( item.flags() );
  if ( status.isRead() || status.isSpam() || status.isIgnored() )
    return;

  m_filterManager->process( item, mRequestedPart, FilterManager::Inbound, true, collection.resource() );
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

void MailFilterAgent::mailCollectionRemoved( const Akonadi::Collection& collection )
{
  changeRecorder()->setCollectionMonitored( collection, false );
  m_filterManager->mailCollectionRemoved(collection);
}

QString MailFilterAgent::createUniqueName( const QString &nameTemplate )
{
  return m_filterManager->createUniqueName( nameTemplate );
}

void MailFilterAgent::filterItems( const QVector<qlonglong> &itemIds, int filterSet )
{
  QList<Akonadi::Item> items;
  foreach ( qlonglong id, itemIds ) {
    items << Akonadi::Item( id );
  }

  m_filterManager->applyFilters( items, static_cast<FilterManager::FilterSet>(filterSet) );
}

void MailFilterAgent::applySpecificFilters( const QVector<qlonglong> &itemIds, int requires, const QStringList& listFilters )
{
  QList<Akonadi::Item> items;
  foreach ( qlonglong id, itemIds ) {
    items << Akonadi::Item( id );
  }

  m_filterManager->applySpecificFilters( items, static_cast<MailCommon::SearchRule::RequiredPart>(requires),listFilters );
}


void MailFilterAgent::filterItem( qlonglong item, int filterSet, const QString &resourceId )
{
  m_filterManager->filter( item, static_cast<FilterManager::FilterSet>( filterSet ), resourceId );
}

void MailFilterAgent::filter(qlonglong item, const QString &filterIdentifier , int requires)
{
  m_filterManager->filter( item, filterIdentifier, static_cast<MailCommon::SearchRule::RequiredPart>(requires) );
}

void MailFilterAgent::reload()
{
  m_filterManager->readConfig();
}

void MailFilterAgent::showFilterLogDialog(qlonglong windowId)
{
  if ( !m_filterLogDialog ) {
    m_filterLogDialog = new FilterLogDialog( 0 );
  }
#ifndef Q_WS_WIN
  KWindowSystem::setMainWindow(m_filterLogDialog,windowId);
#else
  KWindowSystem::setMainWindow(m_filterLogDialog,(HWND)windowId);
#endif
  m_filterLogDialog->show();
  m_filterLogDialog->raise();
  m_filterLogDialog->activateWindow();
  m_filterLogDialog->setModal(false);
}

AKONADI_AGENT_MAIN( MailFilterAgent )

#include "mailfilteragent.moc"
