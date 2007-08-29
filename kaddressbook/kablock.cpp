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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kabc/addressbook.h>
#include <kabc/resource.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "kablock.h"

class KABLockHelper {
  public:
    KABLockHelper() : q( 0 ) {}
    ~KABLockHelper() { delete q; }
    KABLock *q;
};

K_GLOBAL_STATIC( KABLockHelper, s_globalKABLock );

class AddressBookWrapper : public KABC::AddressBook
{
  public:
    AddressBookWrapper( KABC::AddressBook* );

    KABC::Resource* getStandardResource()
    {
      return standardResource();
    }
};

KABLock::KABLock( KABC::AddressBook *ab )
  : mAddressBook( ab )
{
  Q_ASSERT( !( s_globalKABLock->q ) );
  s_globalKABLock->q = this;
}

KABLock::~KABLock()
{
}

KABLock *KABLock::self( KABC::AddressBook *ab )
{
  if ( !s_globalKABLock->q )
    new KABLock( ab );
  else
    s_globalKABLock->q->mAddressBook = ab;

  return s_globalKABLock->q;
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
  AddressBookWrapper *wrapper = static_cast<AddressBookWrapper*>( mAddressBook );
  if ( resource == 0 )
    resource = wrapper->getStandardResource();

  if ( mLocks.find( resource ) == mLocks.end() ) { // hmm, not good...
    return false;
  } else {
    LockEntry &entry = mLocks[ resource ];
    entry.counter--;

    if ( entry.counter == 0 ) {
      mAddressBook->save( entry.ticket );
//      # Activate in KDE 4.0
//      mAddressBook->releaseSaveTicket( entry.ticket );

      mLocks.remove( resource );
    }
  }

  return true;
}
