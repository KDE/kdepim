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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "simpleaddresseeeditor.h"

#include <QtGui/QGridLayout>
#include <QtGui/QLabel>

#include <kdialog.h>
#include <klineedit.h>
#include <klocale.h>

SimpleAddresseeEditor::SimpleAddresseeEditor( QWidget *parent )
  : AddresseeEditorBase( parent ),
    mDirty( false ),
    mBlockModified( false )
{
  kDebug(5720) <<"SimpleAddresseeEditor()";

  initGui();
}

SimpleAddresseeEditor::~SimpleAddresseeEditor()
{
  kDebug(5720) <<"~SimpleAddresseeEditor()";
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
  QGridLayout *topLayout = new QGridLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );
  topLayout->setMargin( 0 );

  QLabel *label = new QLabel( i18n( "Name:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mNameEdit = new KLineEdit( this );
  topLayout->addWidget( mNameEdit, 0, 1 );
  connect( mNameEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( emitModified() ) );

  label = new QLabel( i18n( "Email:" ), this );
  topLayout->addWidget( label, 1, 0 );

  mEmailEdit = new KLineEdit( this );
  topLayout->addWidget( mEmailEdit, 1, 1 );
  connect( mEmailEdit, SIGNAL( textChanged( const QString & ) ),
           SLOT( emitModified() ) );
}

void SimpleAddresseeEditor::load()
{
  kDebug(5720) <<"SimpleAddresseeEditor::load()";

  kDebug(5720) <<"ASSEMBLED NAME:" << mAddressee.assembledName();
  kDebug(5720) <<"EMAIL NAME:" << mAddressee.preferredEmail();

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

void SimpleAddresseeEditor::emitModified()
{
  if ( mBlockModified )
    return;

  mDirty = true;

  emit modified();
}

#include "simpleaddresseeeditor.moc"
