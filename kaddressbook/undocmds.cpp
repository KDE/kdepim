/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>
                  2005 Tobias Koenig <tokoe@kde.org>

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

#include <qapplication.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kapplication.h>

#include "addresseeutil.h"
#include "addresseeconfig.h"
#include "core.h"
#include "kablock.h"

#include "undocmds.h"

bool Command::resourceExist( KABC::Resource *resource )
{
  QPtrList<KABC::Resource>  lst = addressBook()->resources();
  for ( Resource *res = lst.first(); res; res = lst.next() ) {
    if ( res == resource )
      return true;
  }
  return false;
}

DeleteCommand::DeleteCommand( KABC::AddressBook *addressBook,
                              const QStringList &uidList)
  : Command( addressBook ), mUIDList( uidList )
{
}

QString DeleteCommand::name() const
{
  return i18n( "Delete Contact", "Delete %n Contacts", mUIDList.count() );
}

void DeleteCommand::unexecute()
{
  // Put it back in the document
  KABC::Addressee::List::ConstIterator it;
  const KABC::Addressee::List::ConstIterator endIt( mAddresseeList.end() );

  // lock resources
  for ( it = mAddresseeList.begin(); it != endIt; ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) )
      addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  mAddresseeList.clear();
}

void DeleteCommand::execute()
{
  KABC::Addressee addr;

  QStringList::ConstIterator it;
  const QStringList::ConstIterator endIt( mUIDList.end() );
  for ( it = mUIDList.begin(); it != endIt; ++it ) {
    addr = addressBook()->findByUid( *it );
    lock()->lock( addr.resource() );
    mAddresseeList.append( addr );
    AddresseeConfig cfg( addr );
    cfg.remove();
  }

  KABC::Addressee::List::ConstIterator addrIt;
  const KABC::Addressee::List::ConstIterator addrEndIt( mAddresseeList.end() );
  for ( addrIt = mAddresseeList.begin(); addrIt != addrEndIt; ++addrIt ) {
    if ( resourceExist( ( *addrIt ).resource() ) )
      addressBook()->removeAddressee( *addrIt );
    lock()->unlock( (*addrIt).resource() );
  }
}


PasteCommand::PasteCommand( KAB::Core *core, const KABC::Addressee::List &addressees )
  : Command( core->addressBook() ), mAddresseeList( addressees ), mCore( core )
{
}

QString PasteCommand::name() const
{
  return i18n( "Paste Contact", "Paste %n Contacts", mAddresseeList.count() );
}

void PasteCommand::unexecute()
{
  KABC::Addressee::List::ConstIterator it;
  const KABC::Addressee::List::ConstIterator endIt( mAddresseeList.end() );

  // lock resources
  for ( it = mAddresseeList.begin(); it != endIt; ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) )
      addressBook()->removeAddressee( *it );
    lock()->unlock( (*it).resource() );
  }
}

void PasteCommand::execute()
{
  QStringList uids;

  KABC::Addressee::List::ConstIterator constIt;
  const KABC::Addressee::List::ConstIterator constEndIt( mAddresseeList.end() );

  // lock resources
  for ( constIt = mAddresseeList.begin(); constIt != constEndIt; ++constIt )
    lock()->lock( (*constIt).resource() );

  KABC::Addressee::List::Iterator it;
  const KABC::Addressee::List::Iterator endIt( mAddresseeList.end() );
  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) ) {

      /**
         We have to set a new uid for the contact, otherwise insertAddressee()
         ignore it.
      */
      (*it).setUid( KApplication::randomString( 10 ) );
      uids.append( (*it).uid() );
      addressBook()->insertAddressee( *it );
    }
    lock()->unlock( (*it).resource() );
  }

}


NewCommand::NewCommand( KABC::AddressBook *addressBook, const KABC::Addressee::List &addressees )
  : Command( addressBook ), mAddresseeList( addressees )
{
}

QString NewCommand::name() const
{
  return i18n( "New Contact", "New %n Contacts", mAddresseeList.count() );
}

void NewCommand::unexecute()
{
  KABC::Addressee::List::ConstIterator it;
  const KABC::Addressee::List::ConstIterator endIt( mAddresseeList.end() );

  // lock resources
  for ( it = mAddresseeList.begin(); it != endIt; ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) )
      addressBook()->removeAddressee( *it );
    lock()->unlock( (*it).resource() );
  }
}

void NewCommand::execute()
{
  KABC::Addressee::List::Iterator it;
  const KABC::Addressee::List::Iterator endIt( mAddresseeList.end() );

  // lock resources
  for ( it = mAddresseeList.begin(); it != endIt; ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) )
      addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }
}


EditCommand::EditCommand( KABC::AddressBook *addressBook,
                          const KABC::Addressee &oldAddressee,
                          const KABC::Addressee &newAddressee )
  : Command( addressBook ),
    mOldAddressee( oldAddressee ), mNewAddressee( newAddressee )
{
}

QString EditCommand::name() const
{
  return i18n( "Edit Contact" );
}

void EditCommand::unexecute()
{
  if ( resourceExist( mOldAddressee.resource() ) )
  {
    lock()->lock( mOldAddressee.resource() );
    addressBook()->insertAddressee( mOldAddressee );
    lock()->unlock( mOldAddressee.resource() );
  }
}

void EditCommand::execute()
{
  if ( resourceExist( mNewAddressee.resource() ) )
  {
    lock()->lock( mNewAddressee.resource() );
    addressBook()->insertAddressee( mNewAddressee );
    lock()->unlock( mNewAddressee.resource() );
  }
}


CutCommand::CutCommand( KABC::AddressBook *addressBook, const QStringList &uidList )
  : Command( addressBook ), mUIDList( uidList )
{
}

QString CutCommand::name() const
{
  return i18n( "Cut Contact", "Cut %n Contacts", mUIDList.count() );
}

void CutCommand::unexecute()
{
  KABC::Addressee::List::ConstIterator it;
  const KABC::Addressee::List::ConstIterator endIt( mAddresseeList.end() );

  // lock resources
  for ( it = mAddresseeList.begin(); it != endIt; ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
    if ( resourceExist( ( *it ).resource() ) )
      addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  mAddresseeList.clear();

  QClipboard *cb = QApplication::clipboard();
  kapp->processEvents();
  cb->setText( mOldText );
}

void CutCommand::execute()
{
  KABC::Addressee addr;

  QStringList::ConstIterator it;
  const QStringList::ConstIterator endIt( mUIDList.end() );
  for ( it = mUIDList.begin(); it != endIt; ++it ) {
    addr = addressBook()->findByUid( *it );
    mAddresseeList.append( addr );
    lock()->lock( addr.resource() );
  }

  KABC::Addressee::List::ConstIterator addrIt;
  const KABC::Addressee::List::ConstIterator addrEndIt( mAddresseeList.end() );
  for ( addrIt = mAddresseeList.begin(); addrIt != addrEndIt; ++addrIt ) {
    if ( resourceExist( ( *addrIt ).resource() ) )
      addressBook()->removeAddressee( *addrIt );
    lock()->unlock( addr.resource() );
  }

  // Convert to clipboard
  mClipText = AddresseeUtil::addresseesToClipboard( mAddresseeList );

  QClipboard *cb = QApplication::clipboard();
  mOldText = cb->text();
  kapp->processEvents();
  cb->setText( mClipText );
}

CopyToCommand::CopyToCommand( KABC::AddressBook *addressBook, const QStringList &uidList,
                                              KABC::Resource *resource )
    : Command( addressBook ), mUIDList( uidList ), mResource( resource )
{
}

QString CopyToCommand::name() const
{
    return i18n( "Copy Contact To", "Copy %n Contacts To", mUIDList.count() );
}

void CopyToCommand::unexecute()
{
    KABC::Addressee::List::ConstIterator it;
    const KABC::Addressee::List::ConstIterator endIt( mAddresseeList.end() );
    //For copy : just remove it from the "copied to" resource.
    // lock resources
    for ( it = mAddresseeList.begin(); it != endIt; ++it )
        lock()->lock( (*it).resource() );

    for ( it = mAddresseeList.begin(); it != endIt; ++it ) {
      if ( resourceExist( ( *it ).resource() ) )
        addressBook()->removeAddressee( *it );
      lock()->unlock( (*it).resource() );
    }
}

void CopyToCommand::execute()
{
  KABLock::self( addressBook() )->lock( mResource );
  QStringList::Iterator it( mUIDList.begin() );
  const QStringList::Iterator endIt( mUIDList.end() );
  while ( it != endIt ) {
    KABC::Addressee addr = addressBook()->findByUid( *it++ );
    if ( !addr.isEmpty() ) {
      KABC::Addressee newAddr( addr );
      // We need to set a new uid, otherwise the insert below is
      // ignored. This is bad for syncing, but unavoidable, afaiks
      newAddr.setUid( KApplication::randomString( 10 ) );
      newAddr.setResource( mResource );
      if ( resourceExist( newAddr.resource() ) )
        addressBook()->insertAddressee( newAddr );
      mAddresseeList.append( newAddr );
    }
  }
  KABLock::self( addressBook() )->unlock( mResource );

}

MoveToCommand::MoveToCommand( KAB::Core *core, const QStringList &uidList,
                                              KABC::Resource *resource )
    : Command( core->addressBook() ), mUIDList( uidList ), mResource( resource ), mCore( core )
{
}

QString MoveToCommand::name() const
{
    return i18n( "Move Contact To", "Move %n Contacts To", mUIDList.count() );
}

void MoveToCommand::unexecute()
{
  //For move : remove it from the "copied to" resource and insert it back to "copied from" resource.
    KABC::Resource *resource = mCore->requestResource( mCore->widget() );
    if ( !resource )
      return;
    moveContactTo( resource );
}

void MoveToCommand::execute()
{
    moveContactTo( mResource );
}

void MoveToCommand::moveContactTo( KABC::Resource *resource )
{
    KABLock::self( addressBook() )->lock( resource );
    QStringList::Iterator it( mUIDList.begin() );
    const QStringList::Iterator endIt( mUIDList.end() );
    while ( it != endIt ) {
        KABC::Addressee addr = addressBook()->findByUid( *it++ );
        if ( !addr.isEmpty() ) {
            KABC::Addressee newAddr( addr );
      // We need to set a new uid, otherwise the insert below is
      // ignored. This is bad for syncing, but unavoidable, afaiks
            QString uid = KApplication::randomString( 10 );
            newAddr.setUid( uid );
            newAddr.setResource( resource );
            if ( resourceExist( newAddr.resource() ) )
              addressBook()->insertAddressee( newAddr );
            mAddresseeList.append( newAddr );
            mUIDList.append( uid );
            const bool inserted = addressBook()->find( newAddr ) != addressBook()->end();
            if ( inserted ) {
              if ( resourceExist( addr.resource() ) ) {
                KABLock::self( addressBook() )->lock( addr.resource() );
                addressBook()->removeAddressee( addr );
                KABLock::self( addressBook() )->unlock( addr.resource() );
              }
            }
        }
    }
    KABLock::self( addressBook() )->unlock( resource );

}
