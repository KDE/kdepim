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
#include "archivemailagentadaptor.h"
#include "archivemaildialog.h"
#include "archivemailmanager.h"

#include <mailcommon/mailkernel.h>
#include <akonadi/dbusconnectionpool.h>
#include <Akonadi/Monitor>
#include <Akonadi/Session>
#include <Akonadi/CollectionFetchScope>
#include <KMime/Message>
#include <KWindowSystem>

#include <QTimer>

//#define DEBUG_ARCHIVEMAILAGENT 1

ArchiveMailAgent::ArchiveMailAgent( const QString &id )
  : Akonadi::AgentBase( id )
{
  mArchiveManager = new ArchiveMailManager(this);
  KGlobal::locale()->insertCatalog( "libmailcommon" );

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
#ifdef DEBUG_ARCHIVEMAILAGENT
  QTimer::singleShot(1000,mArchiveManager,SLOT(load()));
#else
  QTimer::singleShot(1000*60*5,mArchiveManager,SLOT(load()));
#endif
  mTimer = new QTimer(this);
  connect(mTimer, SIGNAL(timeout()), this, SLOT(reload()));
  mTimer->start(24*60*60*1000);
}

ArchiveMailAgent::~ArchiveMailAgent()
{
}

void ArchiveMailAgent::mailCollectionRemoved(const Akonadi::Collection& collection)
{
  mArchiveManager->removeCollection(collection);
}

void ArchiveMailAgent::showConfigureDialog(qlonglong windowId)
{
  ArchiveMailDialog *dialog = new ArchiveMailDialog();
  if(windowId) {
#ifndef Q_WS_WIN
    KWindowSystem::setMainWindow( dialog, windowId );
#else
    KWindowSystem::setMainWindow( dialog, (HWND)windowId );
#endif
  }
  if(dialog->exec()) {
    mArchiveManager->load();
  }
  delete dialog;
}


void ArchiveMailAgent::reload()
{
  mArchiveManager->load();
  mTimer->start();
}

void ArchiveMailAgent::configure( WId windowId )
{
  showConfigureDialog(windowId);
}

void ArchiveMailAgent::pause()
{
  mArchiveManager->pause();
}

void ArchiveMailAgent::resume()
{
  mArchiveManager->resume();
}


AKONADI_AGENT_MAIN( ArchiveMailAgent )

#include "archivemailagent.moc"
