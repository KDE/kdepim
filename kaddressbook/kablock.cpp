/*
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <klocale.h>
#include <kmessagebox.h>
#include <kstaticdeleter.h>

#include "kablock.h"

KABLock *KABLock::mSelf = 0;

static KStaticDeleter<KABLock> kabLockDeleter;

KABLock::KABLock( KABC::AddressBook *ab )
  : mAddressBook( ab )
{
}

KABLock::~KABLock()
{
}

KABLock *KABLock::self( KABC::AddressBook *ab )
{
  if ( !mSelf )
    kabLockDeleter.setObject( mSelf, new KABLock( ab ) );

  return mSelf;
}

bool KABLock::lock( KABC::Resource *resource )
{
  if ( mLocks.find( resource ) == mLocks.end() ) { // not locked yet
    KABC::Ticket *ticket = mAddressBook->requestSaveTicket( resource );
    if ( !ticket ) {
      return false;
    } else {
      LockEntry entry;
      entry.ticket = ticket;
      entry.counter = 1;
      mLocks.insert( resource, entry );
    }
  } else {
    LockEntry &entry = mLocks[ resource ];
    entry.counter++;
  }

  return true;
}

bool KABLock::unlock( KABC::Resource *resource )
{
  if ( mLocks.find( resource ) == mLocks.end() ) { // hmm, not good...
    return false;
  } else {
    LockEntry &entry = mLocks[ resource ];
    entry.counter--;

    if ( entry.counter == 0 ) {
      mAddressBook->save( entry.ticket );
      mAddressBook->releaseSaveTicket( entry.ticket );

      mLocks.remove( resource );
    }
  }

  return true;
}
