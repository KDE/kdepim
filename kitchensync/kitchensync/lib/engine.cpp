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

#include "engine.h"

#include "actionpart.h"

#include <konnector.h>
#include <konnectormanager.h>
#include <konnectorinfo.h>
#include <klocale.h>

#include <qdatetime.h>

using namespace KSync;

Engine::Engine( QPtrList<ActionPart> &parts )
  : mParts( parts )
{
}

Engine::~Engine()
{
}

void Engine::logMessage( const QString &message )
{
  QString text = QTime::currentTime().toString() + ": ";
  text += message;

  kdDebug() << "LOG: " << text << endl;
}

Konnector::List Engine::konnectors()
{
  return mKonnectors;
}

void Engine::go()
{
  kdDebug() << "Engine::go():" << endl;

  logMessage( i18n("Sync Action triggered") );

  mOpenedKonnectors.clear();
  mProcessedKonnectors.clear();
  mKonnectorCount = 0;

  mKonnectors.clear();

  KRES::Manager<Konnector> *manager = KonnectorManager::self();
  KRES::Manager<Konnector>::ActiveIterator it;
  for( it = manager->activeBegin(); it != manager->activeEnd(); ++it ) {
    kdDebug() << "  Engine::go(): Konnector: id: " << (*it)->identifier()
              << " name: " << (*it)->resourceName() << endl;
    mKonnectors.append( *it );
  }

  bool needsRead = false;

  ActionPart *part;
  for ( part = mParts.first(); part; part = mParts.next() ) {
    part->filterKonnectors( mKonnectors );
    if ( part->needsKonnectorRead() ) needsRead = true;
  }

  if ( needsRead ) {
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
  } else {
    executeActions();
  }
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

  if ( mKonnectorCount == mProcessedKonnectors.count() ) {
    executeActions();
  }
}

void Engine::executeActions()
{
  logMessage( i18n("Execute Actions") );

  /*
   * Apply filters before sync
   */
  Konnector *konnector;
  for ( konnector = mOpenedKonnectors.first(); konnector;
        konnector = mOpenedKonnectors.next() )
    konnector->applyFilters( KSync::Konnector::FilterBeforeSync );

  bool needsWrite = false;

  ActionPart *part;
  for ( part = mParts.first(); part; part = mParts.next() ) {
    part->executeAction();
    if ( part->needsKonnectorWrite() ) needsWrite = true;
  }

  if ( needsWrite ) {
    mProcessedKonnectors.clear();

    for( konnector = mOpenedKonnectors.first(); konnector;
         konnector = mOpenedKonnectors.next() ) {
      konnector->applyFilters( KSync::Konnector::FilterAfterSync );
      if ( konnector->writeSyncees() ) {
        kdDebug() << "writeSyncees(): " << konnector->resourceName() << endl;
      } else {
        kdError() << "Error requesting to write Syncee: "
                  << konnector->resourceName() << endl;
      }
    }
  } else {
    finish();
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

#include "engine.moc"
