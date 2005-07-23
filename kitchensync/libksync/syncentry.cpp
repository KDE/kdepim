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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "syncentry.h"

#include "merger.h"
#include "syncee.h"

#include <kdebug.h>


using namespace KSync;

SyncEntry::SyncEntry( Syncee *sync )
  : mSyncee( sync ), mDontSync( false )
{
  mState = Undefined;
}

SyncEntry::SyncEntry( const SyncEntry &ent )
{
  mState    = ent.mState;
  mSyncee   = ent.mSyncee;
  mDontSync = ent.mDontSync;
  mType     = ent.mType;
}

SyncEntry::~SyncEntry()
{
}

void SyncEntry::setSyncee( Syncee *syncee )
{
  mSyncee = syncee;
}

int SyncEntry::match( SyncEntry* /*entry*/ )
{
  return -2;
}

int SyncEntry::compareTo(SyncEntry* /*entry*/ )
{
  return -2;
}

int SyncEntry::state() const
{
  return mState;
}

bool SyncEntry::wasAdded() const
{
  return ( mState == Added );
}

bool SyncEntry::wasModified() const
{
  return ( mState == Modified );
}

bool SyncEntry::wasRemoved() const
{
  return ( mState == Removed );
}

void SyncEntry::setState( int state )
{
  mState = state;
}

void SyncEntry::setSyncState( int state )
{
  mSyncState = state;
}

int SyncEntry::syncState() const
{
  return mSyncState;
}

Syncee *SyncEntry::syncee()const
{
  return mSyncee;
}

void SyncEntry::setDontSync( bool dontSync )
{
  mDontSync = dontSync;
}

bool SyncEntry::dontSync() const
{
  return mDontSync;
}

KSync::Merger* SyncEntry::merger()const {
  if ( !syncee() )
    return 0l;

  return syncee()->merger();
}

bool SyncEntry::mergeWith( SyncEntry* other ) {
  if ( !merger() && !other->merger() )
    return false;

  /* try at least one merger */
  Merger *mer = merger() ? merger() : other->merger();

  return mer ->merge( this, other );
}

KPIM::DiffAlgo* SyncEntry::diffAlgo( SyncEntry*, SyncEntry* )
{
  return 0;
}

/* not implemented here */
void SyncEntry::setId( const QString& )
{
}

QString SyncEntry::type()const
{
  return mType;
}

void SyncEntry::setType( const QString& str )
{
  mType = str;
}
