/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

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

#include <qlayout.h>
#include <qlabel.h>

#include <klineedit.h>
#include <klocale.h>
#include <kdialog.h>

#include "simpleaddresseeeditor.h"

SimpleAddresseeEditor::SimpleAddresseeEditor( KAB::Core *core, bool isExtension,
                                              QWidget *parent, const char *name )
  : AddresseeEditorBase( core, isExtension, parent, name ), mDirty( false ),
    mBlockModified( false )
{
  kdDebug(5720) << "SimpleAddresseeEditor()" << endl;

  initGui();

  // Load the empty addressee as defaults
  load();

  mDirty = false;
}

SimpleAddresseeEditor::~SimpleAddresseeEditor()
{
  kdDebug(5720) << "~SimpleAddresseeEditor()" << endl;
}

void SimpleAddresseeEditor::setAddressee( const KABC::Addressee &addr )
{
  mAddressee = addr;

  load();
}

const KABC::Addressee &SimpleAddresseeEditor::addressee()
{
  return mAddressee;
}

void SimpleAddresseeEditor::setInitialFocus()
{
  mNameEdit->setFocus();
}

void SimpleAddresseeEditor::initGui()
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n("Name:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mNameEdit = new KLineEdit( this );
  topLayout->addWidget( mNameEdit, 0, 1 );
  connect( mNameEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( emitModified() ) );

  label = new QLabel( i18n("Email:" ), this );
  topLayout->addWidget( label, 1, 0 );

  mEmailEdit = new KLineEdit( this );
  topLayout->addWidget( mEmailEdit, 1, 1 );
  connect( mEmailEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( emitModified() ) );
}

void SimpleAddresseeEditor::load()
{
  kdDebug(5720) << "SimpleAddresseeEditor::load()" << endl;

  kdDebug(5720) << "ASSEMBLED NAME: " << mAddressee.assembledName() << endl;
  kdDebug(5720) << "EMAIL NAME: " << mAddressee.preferredEmail() << endl;

  mBlockModified = true;

  mNameEdit->setText( mAddressee.assembledName() );

  mEmailEdit->setText( mAddressee.preferredEmail() );

  mBlockModified = false;

  mDirty = false;
}

void SimpleAddresseeEditor::save()
{
  if ( !mDirty ) return;

  mAddressee.setNameFromString( mNameEdit->text() );
  mAddressee.insertEmail( mEmailEdit->text(), true );

  mDirty = false;
}

bool SimpleAddresseeEditor::dirty()
{
  return mDirty;
}

QString SimpleAddresseeEditor::title() const
{
  return i18n( "Simple Contact Editor" );
}

QString SimpleAddresseeEditor::identifier() const
{
  return "simple_contact_editor";
}

void SimpleAddresseeEditor::emitModified()
{
  mDirty = true;

  KABC::Addressee::List list;

  if ( isExtension() && !mBlockModified ) {
    save();
    list.append( mAddressee );
  }

  emit modified( list );
}

#include "simpleaddresseeeditor.moc"
