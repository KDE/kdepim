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

#include "syncentry.h"

using namespace KSync;

SyncEntry::SyncEntry( Syncee *sync ) :
  mSyncee( sync )
{
    mState = Undefined;
    mSyncState = Undefined;
}

SyncEntry::SyncEntry( const SyncEntry &ent )
{
    kdDebug(5230) << "SyncEntry copy c'tor " << endl;
    mState = ent.mState;
    mSyncee = ent.mSyncee;
    mSyncState = ent.mSyncState;
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
    kdDebug(5230) << "State is " << state << endl;
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

Syncee *SyncEntry::syncee()
{
  return mSyncee;
}

/* not implemented here */
void SyncEntry::setId( const QString& )
{
}

bool SyncEntry::mergeWith( SyncEntry* )
{
    return false;
}
