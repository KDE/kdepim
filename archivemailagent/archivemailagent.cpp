/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "archivemailagent.h"
#include "archivemailkernel.h"
#include "archivemailagentadaptor.h"
#include "archivemaildialog.h"

#include <mailcommon/mailkernel.h>
#include <akonadi/dbusconnectionpool.h>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/CollectionFetchScope>
#include <KMime/Message>

ArchiveMailAgent::ArchiveMailAgent( const QString &id )
  : Akonadi::AgentBase( id )
{
  ArchiveMailKernel *kernel = new ArchiveMailKernel( this );
  CommonKernel->registerKernelIf( kernel ); //register KernelIf early, it is used by the Filter classes
  CommonKernel->registerSettingsIf( kernel ); //SettingsIf is used in FolderTreeWidget

  m_collectionMonitor = new Akonadi::Monitor( this );
  m_collectionMonitor->fetchCollection( true );
  m_collectionMonitor->ignoreSession( Akonadi::Session::defaultSession() );
  m_collectionMonitor->collectionFetchScope().setAncestorRetrieval( Akonadi::CollectionFetchScope::All );
  m_collectionMonitor->setMimeTypeMonitored( KMime::Message::mimeType() );

  new ArchiveMailAgentAdaptor( this );
  Akonadi::DBusConnectionPool::threadConnection().registerObject( QLatin1String( "/ArchiveMailAgent" ), this, QDBusConnection::ExportAdaptors );
  Akonadi::DBusConnectionPool::threadConnection().registerService( QLatin1String( "org.freedesktop.Akonadi.ArchiveMailAgent" ) );
  connect( m_collectionMonitor, SIGNAL(collectionRemoved(Akonadi::Collection)),
           this, SLOT(mailCollectionRemoved(Akonadi::Collection)) );

}

ArchiveMailAgent::~ArchiveMailAgent()
{
}

void ArchiveMailAgent::mailCollectionRemoved(const Akonadi::Collection& collection)
{
  const Akonadi::Collection::Id id = collection.id();
  //TODO remove it from config
  //TODO
}

void ArchiveMailAgent::showConfigureDialog()
{
  ArchiveMailDialog *dialog = new ArchiveMailDialog();
  dialog->exec();
  delete dialog;
  //TODO reload config.
}

void ArchiveMailAgent::configure( WId windowId )
{
  Q_UNUSED( windowId );
  showConfigureDialog();
}

AKONADI_AGENT_MAIN( ArchiveMailAgent )

#include "archivemailagent.moc"
