/*
    This file is part of KAddressBook.
    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include <qlayout.h>
#include <qtimer.h>

#include <klocale.h>

#include "kabprefs.h"

#include "addresseeeditorwidget.h"
#include "simpleaddresseeeditor.h"

#include "addresseeeditorextension.h"

AddresseeEditorExtension::AddresseeEditorExtension( KAB::Core *core, QWidget *parent, const char *name )
  : KAB::ExtensionWidget( core, parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  if ( KABPrefs::instance()->editorType() == KABPrefs::SimpleEditor )
    mAddresseeEditor = new SimpleAddresseeEditor( this );
  else
    mAddresseeEditor = new AddresseeEditorWidget( this );

  layout->addWidget( mAddresseeEditor );
}

AddresseeEditorExtension::~AddresseeEditorExtension()
{
}

void AddresseeEditorExtension::contactsSelectionChanged()
{
  qDebug("void AddresseeEditorExtension::contactsSelectionChanged()" );
  const KABC::Addressee::List selectedAddressees = selectedContacts();
  KABC::Addressee::List modifiedAddress;
  if ( mAddresseeEditor->dirty() ) {
    mAddresseeEditor->save();
    addressees.append( mAddresseeEditor->addressee() );
    modifiedAddress = addressees;
    QTimer::singleShot(0, this, SLOT(emitModifiedAddresses()));
  }

  mAddresseeEditor->setAddressee( selectedAddressees[ 0 ] );
}

void AddresseeEditorExtension::emitModifiedAddresses()
{
  emit modified( addressees );
}

QString AddresseeEditorExtension::title() const
{
  return i18n( "Contact Editor" );
}

QString AddresseeEditorExtension::identifier() const
{
  return "contact_editor";
}

#include "addresseeeditorextension.moc"
