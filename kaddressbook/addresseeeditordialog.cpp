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

#include <qapplication.h>
#include <qlayout.h>

#include <kdebug.h>
#include <klocale.h>

#include "core.h"
#include "addresseeeditorwidget.h"
#include "simpleaddresseeeditor.h"
#include "kabprefs.h"

#include "addresseeeditordialog.h"

AddresseeEditorDialog::AddresseeEditorDialog( KAB::Core *core,
                                              QWidget *parent, const char *name )
  : KDialogBase( KDialogBase::Plain, i18n( "Edit Contact" ),
                 KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Apply,
                 KDialogBase::Ok, parent, name, false )
{
  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setWFlags( getWFlags() | WGroupLeader );

  kdDebug(5720) << "AddresseeEditorDialog()" << endl;

  QWidget *page = plainPage();

  QVBoxLayout *layout = new QVBoxLayout( page );

  if ( KABPrefs::instance()->editorType() == KABPrefs::SimpleEditor ) {
    mEditorWidget = new SimpleAddresseeEditor( core, false, page );
  } else {
    mEditorWidget = new AddresseeEditorWidget( core, false, page );
  }
  connect( mEditorWidget, SIGNAL( modified( const KABC::Addressee::List& ) ),
           SLOT( widgetModified( const KABC::Addressee::List& ) ) );
  layout->addWidget( mEditorWidget );

  enableButton( KDialogBase::Apply, false );

  KConfig config( "kaddressbookrc" );
  config.setGroup( "AddresseeEditor" );
  QSize defaultSize( 750, 570 );
  resize( config.readSizeEntry( "Size", &defaultSize ) );
}

AddresseeEditorDialog::~AddresseeEditorDialog()
{
  kdDebug(5720) << "~AddresseeEditorDialog()" << endl;

  KConfig config( "kaddressbookrc" );
  config.setGroup( "AddresseeEditor" );
  config.writeEntry( "Size", size() );

  emit editorDestroyed( mEditorWidget->addressee().uid() );
}

void AddresseeEditorDialog::setAddressee( const KABC::Addressee &addr )
{
  enableButton( KDialogBase::Apply, false );

  setTitle( addr );

  mEditorWidget->setAddressee( addr );
  mEditorWidget->setInitialFocus();
}

KABC::Addressee AddresseeEditorDialog::addressee()
{
  return mEditorWidget->addressee();
}

bool AddresseeEditorDialog::dirty()
{
  return mEditorWidget->dirty();
}

void AddresseeEditorDialog::slotApply()
{
  if ( mEditorWidget->dirty() ) {
    QApplication::setOverrideCursor( Qt::waitCursor );
    mEditorWidget->save();
    emit contactModified( mEditorWidget->addressee() );
    QApplication::restoreOverrideCursor();
  }

  enableButton( KDialogBase::Apply, false );

  KDialogBase::slotApply();
}

void AddresseeEditorDialog::slotOk()
{
  slotApply();

  KDialogBase::slotOk();

  // Destroy this dialog
  delayedDestruct();
}

void AddresseeEditorDialog::widgetModified( const KABC::Addressee::List &addressees )
{
  if ( !addressees.isEmpty() )
    setTitle( addressees.first() );

  enableButton( KDialogBase::Apply, true );
}

void AddresseeEditorDialog::slotCancel()
{
  KDialogBase::slotCancel();

  // Destroy this dialog
  delayedDestruct();
}

void AddresseeEditorDialog::setTitle( const KABC::Addressee &addr )
{
  if ( !addr.realName().isEmpty() )
    setCaption( i18n( "Edit Contact '%1'" ).arg( addr.realName() ) );
}

#include "addresseeeditordialog.moc"
