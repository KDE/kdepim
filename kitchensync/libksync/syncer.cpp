/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>

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

#include <kdebug.h>

#include "standardsync.h"
#include "syncui.h"
#include "syncee.h"

#include "syncer.h"

using namespace KSync;

Syncer::Syncer( SyncUi *ui, SyncAlgorithm *algorithm )
{
//  mSyncees.setAutoDelete(true); this leads to crashes
  if ( !ui ) mUi = new SyncUi();
  else mUi = ui;

  if ( !algorithm ) mAlgorithm = new StandardSync( mUi );
  else mAlgorithm = algorithm;
}

Syncer::~Syncer()
{
  delete mUi;
  delete mAlgorithm;
}

void Syncer::addSyncee( Syncee *syncee )
{
  mSyncees.append( syncee );
}

void Syncer::clear()
{
  mSyncees.clear();
}

void Syncer::sync()
{
  Syncee *target = mSyncees.last();

  if ( !target ) {
    kdWarning() << "Syncer::sync(): No Syncees set." << endl;
    return;
  }

  Syncee *syncee = mSyncees.first();
  while ( syncee != target ) {
    syncToTarget( syncee, target );
    syncee = mSyncees.next();
  }
  if ( !target->saveLog() ) {
    kdDebug() << "Syncer::sync() failed to save target log." << endl;
  }
  syncee = mSyncees.first();
  while ( syncee != target ) {
    syncToTarget( target, syncee, true );
    if ( !syncee->saveLog() ) {
      kdDebug() << "Syncer::sync() failed to save syncee log." << endl;
    }
    syncee = mSyncees.next();
  }
}

void Syncer::syncAllToTarget( Syncee *target, bool writeback )
{
  Syncee *syncee = mSyncees.first();
  while ( syncee ) {
    syncToTarget(syncee,target);
    syncee = mSyncees.next();
  }

  if ( !target->saveLog() ) {
    kdDebug() << "Syncer::syncAllToTarget() failed to save target log." << endl;
  }

  if ( writeback ) {
    for ( Syncee *syncee = mSyncees.first(); syncee;
          syncee = mSyncees.next() ) {
      syncToTarget( target, syncee, true );
    }
  }
}

void Syncer::syncToTarget( Syncee *source, Syncee *target, bool override )
{
  mAlgorithm->syncToTarget( source, target, override );
}

void Syncer::setSyncAlgorithm( SyncAlgorithm *algorithm )
{
  if ( !algorithm ) return;
  delete mAlgorithm;
  mAlgorithm = algorithm;
}
