/*
    This file is part of KitchenSync.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <konnector.h>
#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <klocale.h>

#include <addressbooksyncee.h>
#include <calendarsyncee.h>
#include <synceelist.h>
#include <syncuikde.h>


#include <qdatetime.h>

#include "konnectorpair.h"
#include "stdsyncui.h"

#include "engine.h"

using namespace KSync;

Engine::Engine()
  : mSyncUi( 0 )
{
}

Engine::~Engine()
{
  delete mSyncUi;
  mSyncUi = 0;
}

void Engine::logMessage( const QString &message )
{
  QString text = QTime::currentTime().toString() + ": ";
  text += message;

  kdDebug() << "LOG: " << text << endl;
}

void Engine::setResolveStrategy( int strategy )
{
  delete mSyncUi;

  switch ( strategy ) {
    case KonnectorPair::ResolveFirst:
      mSyncUi = new SyncUiFirst();
      break;
    case KonnectorPair::ResolveSecond:
      mSyncUi = new SyncUiSecond();
      break;
    case KonnectorPair::ResolveBoth:
      mSyncUi = new KSync::SyncUi();
      break;
    default:
      mSyncUi = new SyncUiKde( 0, true, true );
  }

  mCalendarSyncer.setSyncUi( mSyncUi );
  mAddressBookSyncer.setSyncUi( mSyncUi );
}

void Engine::go( KonnectorPair *pair )
{
  kdDebug() << "Engine::go():" << endl;

  logMessage( i18n("Sync Action triggered") );

  setResolveStrategy( pair->resolveStrategy() );

  mOpenedKonnectors.clear();
  mProcessedKonnectors.clear();
  mKonnectorCount = 0;

  mKonnectors.clear();

  KonnectorManager *manager = pair->manager();
  KonnectorManager::Iterator it;
  for ( it = manager->begin(); it != manager->end(); ++it )
    mKonnectors.append( *it );

  Konnector *k;
  for( k = mKonnectors.first(); k; k = mKonnectors.next() ) {
    logMessage( i18n("Connecting '%1'").arg( k->resourceName() ) );
    if ( !k->connectDevice() ) {
      logMessage( i18n("Error connecting device.") );
    } else {
      mOpenedKonnectors.append( k );
      ++mKonnectorCount;
    }
  }

  for ( k = mOpenedKonnectors.first(); k; k = mOpenedKonnectors.next() ) {
    logMessage( i18n("Request Syncees") );
    if ( !k->readSyncees() ) {
      logMessage( i18n("Request failed.") );
    }
  }

  qDebug( "|||||||||||<<use %d konnectors", mKonnectorCount );
}

void Engine::slotSynceesRead( Konnector *k )
{
  logMessage( i18n("Syncees read from '%1'").arg( k->resourceName() ) );

  mProcessedKonnectors.append( k );

  SynceeList syncees = k->syncees();

  if ( syncees.count() == 0 ) {
    logMessage( i18n("Syncee list is empty.") );
    return;
  }

  tryExecuteActions();
}

void Engine::tryExecuteActions()
{
  kdDebug() << "Engine::tryExecuteActions()" << endl;


  kdDebug() << "  konnectorCount: " << mKonnectorCount << endl;
  kdDebug() << "  processedKonnectorsCount: " << mProcessedKonnectors.count()
            << endl;

  Konnector *k;
  for( k = mProcessedKonnectors.first(); k; k = mProcessedKonnectors.next() )
    logMessage( i18n("Processed '%1'").arg( k->resourceName() ) );

  if ( mKonnectorCount == mProcessedKonnectors.count() ) {
    executeActions();
  }
}

void Engine::executeActions()
{
  logMessage( i18n("Execute Actions") );

  doSync();

  mProcessedKonnectors.clear();

  Konnector *konnector;
  for( konnector = mOpenedKonnectors.first(); konnector;
       konnector = mOpenedKonnectors.next() ) {
    if ( konnector->writeSyncees() ) {
      kdDebug() << "writeSyncees(): " << konnector->resourceName() << endl;
    } else {
      kdError() << "Error requesting to write Syncee: "
                << konnector->resourceName() << endl;
    }
  }
}

void Engine::slotSynceeReadError( Konnector *k )
{
  logMessage( i18n("Error reading Syncees from '%1'")
              .arg( k->resourceName() ) );

  --mKonnectorCount;

  tryExecuteActions();
}

void Engine::slotSynceesWritten( Konnector *k )
{
  logMessage( i18n("Syncees written to '%1'").arg( k->resourceName() ) );

  mProcessedKonnectors.append( k );

  disconnectDevice( k );

  tryFinish();
}

void Engine::slotSynceeWriteError( Konnector *k )
{
  logMessage( i18n("Error writing Syncees to '%1'")
              .arg( k->resourceName() ) );

  --mKonnectorCount;

  disconnectDevice( k );

  tryFinish();
}

void Engine::disconnectDevice( Konnector *k )
{
  if ( !k->disconnectDevice() ) {
    logMessage( i18n("Error disconnecting device") );
  }
}

void Engine::tryFinish()
{
  if ( mKonnectorCount == mProcessedKonnectors.count() ) {
    finish();
  }
}

void Engine::finish()
{
  logMessage( i18n("Synchronization finished.") );
  emit doneSync();
}

void Engine::doSync()
{
  mCalendarSyncer.clear();
  mAddressBookSyncer.clear();

  Konnector *k;
  for( k = mKonnectors.first(); k; k = mKonnectors.next() ) {
    qDebug( "adding syncees for %s", k->resourceName().latin1() );
    SynceeList syncees = k->syncees();

    if ( syncees.count() == 0 ) {
      qDebug( "Syncee list is empty." );
      continue;
    }

    CalendarSyncee *calendarSyncee = syncees.calendarSyncee();
    if ( calendarSyncee )
      mCalendarSyncer.addSyncee( calendarSyncee );

    AddressBookSyncee *addressBookSyncee = syncees.addressBookSyncee();
    if ( addressBookSyncee )
      mAddressBookSyncer.addSyncee( addressBookSyncee );

    qDebug( "added syncees" );
  }

  mCalendarSyncer.sync();
  mAddressBookSyncer.sync();
}

#include "engine.moc"
