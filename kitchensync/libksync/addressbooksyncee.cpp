/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2004 Cornelius Schumacher <schumacher@kde.org>
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

#include "addressbooksyncee.h"

#include "addressbookmerger.h"
#include "syncee.h"

#include <libkdepim/addresseediffalgo.h>
#include <libkdepim/kabcresourcenull.h>

#include <kapplication.h>
#include <kdebug.h>
#include <kstaticdeleter.h>

using namespace KSync;

AddressBookSyncEntry::AddressBookSyncEntry( Syncee* parent )
  : SyncEntry( parent )
{
  setType( QString::fromLatin1("AddressBookSyncEntry") );
}

AddressBookSyncEntry::AddressBookSyncEntry( const KABC::Addressee &a,
                                            Syncee *parent )
  : SyncEntry( parent )
{
  mAddressee = a;
  setType( QString::fromLatin1("AddressBookSyncEntry") );
}

AddressBookSyncEntry::AddressBookSyncEntry( const AddressBookSyncEntry& entry )
  : SyncEntry( entry )
{
  mAddressee = entry.mAddressee;
  m_res = entry.m_res;
  //  type is copied by the SyncEntry c'tor
}

QString AddressBookSyncEntry::name()
{
  return mAddressee.realName();
}

QString AddressBookSyncEntry::id()
{
  return mAddressee.uid();
}

void AddressBookSyncEntry::setId(const QString& id)
{
  mAddressee.setUid( id );
}

AddressBookSyncEntry* AddressBookSyncEntry::clone() {
    return new AddressBookSyncEntry( *this );
}

QString AddressBookSyncEntry::timestamp()
{
  QDateTime r = mAddressee.revision();
  if ( r.isValid() ) return r.toString();
  else return "norevision";
}

bool AddressBookSyncEntry::equals( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::equals(): Wrong type." << endl;
    return false;
  }

  if ( mAddressee == abEntry->addressee() ) {
    kdDebug(5228) << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "equal" << endl;
    return true;
  } else {
    kdDebug(5228) << "AddressBookSyncEntry::equals(): '" << entry->name() << "':"
              << "not equal" << endl;
    return false;
  }
}

QString AddressBookSyncEntry::resource() const
{
  return m_res;
}

void AddressBookSyncEntry::setResource( const QString &str )
{
  m_res = str;
}

KPIM::DiffAlgo* AddressBookSyncEntry::diffAlgo( SyncEntry *syncEntry, SyncEntry *targetEntry )
{
  AddressBookSyncEntry *abSyncEntry = dynamic_cast<AddressBookSyncEntry*>( syncEntry );
  AddressBookSyncEntry *abTargetEntry = dynamic_cast<AddressBookSyncEntry*>( targetEntry );

  if ( !abSyncEntry || !abTargetEntry )
    return 0;

  return new KPIM::AddresseeDiffAlgo( abSyncEntry->addressee(), abTargetEntry->addressee() );
}

void AddressBookSyncEntry::setAddressee( const KABC::Addressee& addr ) {
  mAddressee = addr;
}


/////////////////
///// Syncee Implementation
/////
AddressBookSyncee::AddressBookSyncee( AddressBookMerger* merger)
  : Syncee( merger )
{
  setType( QString::fromLatin1("AddressBookSyncee") );
  mAddressBook = new KABC::AddressBook;
  mAddressBook->addResource( new KABC::ResourceNull() );
  mOwnAddressBook = true;

  mEntries.setAutoDelete( false );
}

AddressBookSyncee::AddressBookSyncee( KABC::AddressBook *ab, AddressBookMerger* merger )
  : Syncee( merger ) // set the support size
{
  setType( QString::fromLatin1("AddressBookSyncee") );
  mAddressBook = ab;
  mOwnAddressBook = false;

  mEntries.setAutoDelete( false );

  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it )
    createEntry( *it );

}

AddressBookSyncee::~AddressBookSyncee()
{
  if ( mOwnAddressBook ) delete mAddressBook;
  mEntries.setAutoDelete( true );
}

void AddressBookSyncee::reset()
{
  mEntries.clear();
}

AddressBookSyncEntry *AddressBookSyncee::firstEntry()
{
  return mEntries.first();
}

AddressBookSyncEntry *AddressBookSyncee::nextEntry()
{
  return mEntries.next();
}

#if 0 // fix me later - zecke
AddressBookSyncEntry *AddressBookSyncee::findEntry(const QString &id)
{
  Event *event = mEntries.find(id);
  return createEntry(event);
}
#endif

void AddressBookSyncee::addEntry( SyncEntry *entry )
{
//  kdDebug() << "AddressBookSyncee::addEntry()" << endl;

  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>( entry );

  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::addEntry(): SyncEntry has wrong type."
                  << endl;
  } else {
    abEntry->setSyncee( this ); // set the parent as we're now responsible

    mEntries.append( abEntry );

    KABC::Addressee a = abEntry->addressee();
    a.setResource( 0 );
    if (!a.revision().isValid() )
      a.setRevision( QDateTime::currentDateTime() );
    mAddressBook->insertAddressee( a );

    /*
     * now we need to update the Addressee to contain the
     * right resource so that removal works as well
     */
    abEntry->setAddressee( mAddressBook->findByUid( a.uid() ) );
  }
}

void AddressBookSyncee::removeEntry( SyncEntry *entry )
{
  AddressBookSyncEntry *abEntry = dynamic_cast<AddressBookSyncEntry *>(entry);
  if ( !abEntry ) {
    kdDebug(5228) << "AddressBookSyncee::removeEntry(): SyncEntry has wrong type."
                  << endl;
  } else {
    mAddressBook->removeAddressee( abEntry->addressee() );
    mEntries.remove( abEntry );
  }
}

AddressBookSyncEntry *AddressBookSyncee::createEntry( const KABC::Addressee &a )
{
  if ( !a.isEmpty() ) {
    AddressBookSyncEntry *entry = new AddressBookSyncEntry( a, this );
    entry->setSyncee( this );
    mEntries.append( entry );
    return entry;
  } else {
    return 0;
  }
}


QString AddressBookSyncee::generateNewId() const
{
    return KApplication::randomString( 10 );
}

