/*
    This file is part of ksync.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
#include <kabc/resource.h>
#include <kresources/manager.h>

#include "addressbooksyncee.h"

AddressBookSyncEntry::AddressBookSyncEntry( const KABC::Addressee &a ) :
  mAddressee( a )
{
}

QString AddressBookSyncEntry::name()
{
  return mAddressee.realName();
}

QString AddressBookSyncEntry::id()
{
  return mAddressee.uid();
}

QString AddressBookSyncEntry::timestamp()
{
  return QString::null;
}

bool AddressBookSyncEntry::equals( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug() << "AddressBookSyncee::equals(): Wrong type." << endl;
    return false;
  }

  if ( mAddressee == abEntry->addressee() ) {
    kdDebug() << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "equal" << endl;    
    return true;
  } else {
    kdDebug() << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "not equal" << endl;
    return false;
  }
}

AddressBookSyncee::AddressBookSyncee()
{
  mAddressBook = new KABC::AddressBook;

  mEntries.setAutoDelete(true);
}

AddressBookSyncee::~AddressBookSyncee()
{
  delete mAddressBook;
}

bool AddressBookSyncee::read()
{
  KRES::Manager<KABC::Resource> manager( "contact" );
  KABC::Resource *resource = manager.createResource( "file" );
  mAddressBook->addResource( resource );
  return mAddressBook->load();
}

bool AddressBookSyncee::write()
{
  KABC::Ticket *ticket = mAddressBook->requestSaveTicket();
  if ( !ticket ) return false;
  return mAddressBook->save( ticket );
}


AddressBookSyncEntry *AddressBookSyncee::firstEntry()
{
  mAddressBookIterator = mAddressBook->begin();
  return createEntry( *mAddressBookIterator );
}

AddressBookSyncEntry *AddressBookSyncee::nextEntry()
{
  ++mAddressBookIterator;
  return createEntry( *mAddressBookIterator );
}

#if 0
AddressBookSyncEntry *AddressBookSyncee::findEntry(const QString &id)
{
  Event *event = mAddressBook->getEvent(id);
  return createEntry(event);
}
#endif

void AddressBookSyncee::addEntry( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if (!abEntry) {
    kdDebug() << "AddressBookSyncee::addEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    mAddressBook->insertAddressee( abEntry->addressee() );
  }
}

void AddressBookSyncee::removeEntry( KSyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug() << "AddressBookSyncee::removeEntry(): SyncEntry has wrong type."
              << endl;
  } else {
    mAddressBook->removeAddressee( abEntry->addressee() );
  }
}

AddressBookSyncEntry *AddressBookSyncee::createEntry( const KABC::Addressee &a )
{
  if ( !a.isEmpty() ) {
    AddressBookSyncEntry *entry = new AddressBookSyncEntry( a );
    mEntries.append( entry );
    return entry;
  } else {
    return 0;
  }  
}
