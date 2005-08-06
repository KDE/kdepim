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

/////////////////////////////////
// PwDelete Methods

PwDeleteCommand::PwDeleteCommand( KABC::AddressBook *ab,
                                  const QStringList &uidList)
  : Command( ab ), mAddresseeList(), mUIDList( uidList )
{
}

PwDeleteCommand::~PwDeleteCommand()
{
}

QString PwDeleteCommand::name() const
{
  return i18n( "Delete" );
}

void PwDeleteCommand::unexecute()
{
  // Put it back in the document
  KABC::Addressee::List::ConstIterator it;

  // lock resources
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  mAddresseeList.clear();
}

void PwDeleteCommand::execute()
{
  KABC::Addressee addr;
  KABC::Addressee::List::ConstIterator addrIt;

  QStringList::ConstIterator it;
  for ( it = mUIDList.begin(); it != mUIDList.end(); ++it ) {
    addr = addressBook()->findByUid( *it );
    lock()->lock( addr.resource() );
    mAddresseeList.append( addr );
    AddresseeConfig cfg( addr );
    cfg.remove();
  }

  for ( addrIt = mAddresseeList.begin(); addrIt != mAddresseeList.end(); ++addrIt ) {
    addressBook()->removeAddressee( *addrIt );
    lock()->unlock( (*addrIt).resource() );
  }
}

/////////////////////////////////
// PwPaste Methods

PwPasteCommand::PwPasteCommand( KAB::Core *core,
                                const KABC::Addressee::List &list )
  : Command( core->addressBook() ), mCore( core ), mAddresseeList( list )
{
}

QString PwPasteCommand::name() const
{
  return i18n( "Paste" );
}

void PwPasteCommand::unexecute()
{
  KABC::Addressee::List::ConstIterator it;

  // lock resources
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->removeAddressee( *it );
    lock()->unlock( (*it).resource() );
  }
}

void PwPasteCommand::execute()
{
  QStringList uids;
  KABC::Addressee::List::Iterator it;
  KABC::Addressee::List::ConstIterator constIt;

  // lock resources
  for ( constIt = mAddresseeList.begin(); constIt != mAddresseeList.end(); ++constIt )
    lock()->lock( (*constIt).resource() );

  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    /* we have to set a new uid for the contact, otherwise insertAddressee()
       ignore it.
     */
    (*it).setUid( KApplication::randomString( 10 ) );
    uids.append( (*it).uid() );
    addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  QStringList::ConstIterator uidIt;
  for ( uidIt = uids.begin(); uidIt != uids.end(); ++uidIt )
    mCore->editContact( *uidIt );
}

/////////////////////////////////
// PwNew Methods

PwNewCommand::PwNewCommand( KABC::AddressBook *ab, const KABC::Addressee &addr )
  : Command( ab ), mAddr( addr )
{
}

PwNewCommand::~PwNewCommand()
{
}

QString PwNewCommand::name() const
{
  return i18n( "New Contact" );
}

void PwNewCommand::unexecute()
{
  lock()->lock( mAddr.resource() );
  addressBook()->removeAddressee( mAddr );
  lock()->unlock( mAddr.resource() );
}

void PwNewCommand::execute()
{
  lock()->lock( mAddr.resource() );
  addressBook()->insertAddressee( mAddr );
  lock()->unlock( mAddr.resource() );
}

/////////////////////////////////
// PwEdit Methods

PwEditCommand::PwEditCommand( KABC::AddressBook *ab,
                              const KABC::Addressee &oldAddr,
                              const KABC::Addressee &newAddr )
     : Command( ab ), mOldAddr( oldAddr ), mNewAddr( newAddr )
{
}

PwEditCommand::~PwEditCommand()
{
}

QString PwEditCommand::name() const
{
  return i18n( "Entry Edit" );
}

void PwEditCommand::unexecute()
{
  lock()->lock( mOldAddr.resource() );
  addressBook()->insertAddressee( mOldAddr );
  lock()->unlock( mOldAddr.resource() );
}

void PwEditCommand::execute()
{
  lock()->lock( mNewAddr.resource() );
  addressBook()->insertAddressee( mNewAddr );
  lock()->unlock( mNewAddr.resource() );
}

/////////////////////////////////
// PwCut Methods

PwCutCommand::PwCutCommand( KABC::AddressBook *ab, const QStringList &uidList )
    : Command( ab ), mUIDList( uidList )
{
}

QString PwCutCommand::name() const
{
  return i18n( "Cut" );
}

void PwCutCommand::unexecute()
{
  KABC::Addressee::List::ConstIterator it;

  // lock resources
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->insertAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  mAddresseeList.clear();

  QClipboard *cb = QApplication::clipboard();
  kapp->processEvents();
  cb->setText( mOldText );
}

void PwCutCommand::execute()
{
  KABC::Addressee addr;
  KABC::Addressee::List::ConstIterator addrIt;

  QStringList::ConstIterator it;
  for ( it = mUIDList.begin(); it != mUIDList.end(); ++it ) {
    addr = addressBook()->findByUid( *it );
    mAddresseeList.append( addr );
    lock()->lock( addr.resource() );
  }

  for ( addrIt = mAddresseeList.begin(); addrIt != mAddresseeList.end(); ++addrIt ) {
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
