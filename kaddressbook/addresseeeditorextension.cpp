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

#include "addresseeeditorextension.h"

#include <QtGui/QVBoxLayout>
#include <QTimer>

#include <klocale.h>

#include "addresseeeditorwidget.h"
#include "kabprefs.h"
#include "simpleaddresseeeditor.h"

AddresseeEditorExtension::AddresseeEditorExtension( KAB::Core *core, QWidget *parent )
  : KAB::ExtensionWidget( core, parent )
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
  const KABC::Addressee::List selectedAddressees = selectedContacts();
  KABC::Addressee::List modifiedAddress;
  if ( mAddresseeEditor->dirty() ) {
    mAddresseeEditor->save();
    addressees.append( mAddresseeEditor->addressee() );
    modifiedAddress = addressees;
    QTimer::singleShot(0, this, SLOT(emitModifiedAddresses()));
  }
  if( !selectedAddressees.isEmpty())
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

QSize AddresseeEditorExtension::minimumSizeHint() const
{
  // workaround for this rather large widget also enforcing its minimum size
  // hint in the details stack when not used at all
  if ( !isVisible() )
    return QSize( 0, 0 );
  return KAB::ExtensionWidget::minimumSizeHint();
}

#include "addresseeeditorextension.moc"
