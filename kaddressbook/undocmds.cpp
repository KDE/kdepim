/*
    This file is part of KAddressBook.
    Copyright (C) 1999 Don Sanders <sanders@kde.org>

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

#include <qtextstream.h>
#include <qapplication.h>
#include <qclipboard.h>

#include <klocale.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kabc/addressbook.h>

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
  redo();
}

PwDeleteCommand::~PwDeleteCommand()
{
}

QString PwDeleteCommand::name()
{
  return i18n( "Delete" );
}

bool PwDeleteCommand::undo()
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

  return true;
}

bool PwDeleteCommand::redo()
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

  return true;
}

/////////////////////////////////
// PwPaste Methods

PwPasteCommand::PwPasteCommand( KAB::Core *core,
                                const KABC::Addressee::List &list )
  : Command( core->addressBook() ), mCore( core ), mAddresseeList( list )
{
  redo();
}

QString PwPasteCommand::name()
{
  return i18n( "Paste" );
}

bool PwPasteCommand::undo()
{
  KABC::Addressee::List::ConstIterator it;

  // lock resources
  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it )
    lock()->lock( (*it).resource() );

  for ( it = mAddresseeList.begin(); it != mAddresseeList.end(); ++it ) {
    addressBook()->removeAddressee( *it );
    lock()->unlock( (*it).resource() );
  }

  return true;
}

bool PwPasteCommand::redo()
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

  return true;
}

/////////////////////////////////
// PwNew Methods

PwNewCommand::PwNewCommand( KABC::AddressBook *ab, const KABC::Addressee &addr )
  : Command( ab ), mAddr( addr )
{
  redo();
}

PwNewCommand::~PwNewCommand()
{
}

QString PwNewCommand::name()
{
  return i18n( "New Contact" );
}

bool PwNewCommand::undo()
{
  lock()->lock( mAddr.resource() );
  addressBook()->removeAddressee( mAddr );
  lock()->unlock( mAddr.resource() );

  return true;
}

bool PwNewCommand::redo()
{
  lock()->lock( mAddr.resource() );
  addressBook()->insertAddressee( mAddr );
  lock()->unlock( mAddr.resource() );

  return true;
}

/////////////////////////////////
// PwEdit Methods

PwEditCommand::PwEditCommand( KABC::AddressBook *ab,
                              const KABC::Addressee &oldAddr,
                              const KABC::Addressee &newAddr )
     : Command( ab ), mOldAddr( oldAddr ), mNewAddr( newAddr )
{
  redo();
}

PwEditCommand::~PwEditCommand()
{
}

QString PwEditCommand::name()
{
  return i18n( "Entry Edit" );
}

bool PwEditCommand::undo()
{
  lock()->lock( mOldAddr.resource() );
  addressBook()->insertAddressee( mOldAddr );
  lock()->unlock( mOldAddr.resource() );

  return true;
}

bool PwEditCommand::redo()
{
  lock()->lock( mNewAddr.resource() );
  addressBook()->insertAddressee( mNewAddr );
  lock()->unlock( mNewAddr.resource() );

  return true;
}

/////////////////////////////////
// PwCut Methods

PwCutCommand::PwCutCommand( KABC::AddressBook *ab, const QStringList &uidList )
    : Command( ab ), mUIDList( uidList )
{
  redo();
}

QString PwCutCommand::name()
{
  return i18n( "Cut" );
}

bool PwCutCommand::undo()
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

  return true;
}

bool PwCutCommand::redo()
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

  return true;
}
