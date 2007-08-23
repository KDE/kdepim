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

#include <QApplication>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QFrame>

#include <kdebug.h>
#include <klocale.h>

#include "core.h"
#include "addresseeeditorwidget.h"
#include "simpleaddresseeeditor.h"
#include "kabprefs.h"

#include "addresseeeditordialog.h"

AddresseeEditorDialog::AddresseeEditorDialog( KAB::Core * /*core*/,
                                              QWidget *parent, const char *name )
  : KDialog( parent )
{
  setCaption( i18n( "Edit Contact" ) );
  setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );
  setDefaultButton( KDialog::Ok );

  // Set this to be the group leader for all subdialogs - this means
  // modal subdialogs will only affect this dialog, not the other windows
  setAttribute( Qt::WA_GroupLeader );

  kDebug(5720) <<"AddresseeEditorDialog()";

  QFrame *page = new QFrame( this );
  setMainWidget( page );

  QVBoxLayout *layout = new QVBoxLayout( page );
  layout->setMargin( 0 );
  layout->setSpacing( KDialog::spacingHint() );

  if ( KABPrefs::instance()->editorType() == KABPrefs::SimpleEditor ) {
    mEditorWidget = new SimpleAddresseeEditor( page );
  } else {
    mEditorWidget = new AddresseeEditorWidget( page );
  }
  connect( mEditorWidget, SIGNAL( modified() ), SLOT( widgetModified() ) );
  layout->addWidget( mEditorWidget );

  enableButton( KDialog::Apply, false );

  KConfig _config( "kaddressbookrc" );
  KConfigGroup config(&_config, "AddresseeEditor" );
  QSize defaultSize( 750, 570 );
  resize( config.readEntry( "Size", defaultSize ) );
  connect(this,SIGNAL(okClicked()),SLOT(slotOk()));
  connect(this,SIGNAL(cancelClicked()),SLOT(slotCancel()));
  connect(this,SIGNAL(applyClicked()),SLOT(slotApply()));
}

AddresseeEditorDialog::~AddresseeEditorDialog()
{
  kDebug(5720) <<"~AddresseeEditorDialog()";

  KConfig _config( "kaddressbookrc" );
  KConfigGroup config(&_config, "AddresseeEditor" );
  config.writeEntry( "Size", size() );

  emit editorDestroyed( mEditorWidget->addressee().uid() );
}

void AddresseeEditorDialog::setAddressee( const KABC::Addressee &addr )
{
  enableButton( KDialog::Apply, false );

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
  if ( !mEditorWidget->readyToClose() )
    return;

  if ( mEditorWidget->dirty() ) {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    mEditorWidget->save();
    emit contactModified( mEditorWidget->addressee() );
    QApplication::restoreOverrideCursor();
  }

  enableButton( KDialog::Apply, false );
}

void AddresseeEditorDialog::slotOk()
{
  if ( !mEditorWidget->readyToClose() )
    return;

  slotApply();

  KDialog::accept();

  // Destroy this dialog
  delayedDestruct();
}

void AddresseeEditorDialog::widgetModified()
{
  const KABC::Addressee addressee = mEditorWidget->addressee();
  if ( !addressee.isEmpty() )
    setTitle( addressee );

  enableButton( KDialog::Apply, true );
}

void AddresseeEditorDialog::slotCancel()
{
  KDialog::reject();

  // Destroy this dialog
  delayedDestruct();
}

void AddresseeEditorDialog::setTitle( const KABC::Addressee &addr )
{
  if ( !addr.realName().isEmpty() )
    setCaption( i18n( "Edit Contact '%1'", addr.realName() ) );
}

#include "addresseeeditordialog.moc"
