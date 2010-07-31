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

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqstring.h>
#include <tqtoolbutton.h>
#include <tqtooltip.h>

#include <kaccelmanager.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "imeditwidget.h"
#include "imeditorwidget.h"

IMEditWidget::IMEditWidget( TQWidget *parent, KABC::Addressee &addr, const char *name )
  : TQWidget( parent, name ), mAddressee(addr)
{
  TQGridLayout *topLayout = new TQGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "IM address:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mIMEdit = new KLineEdit( this );
  connect( mIMEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SLOT( textChanged( const TQString& ) ) );
  connect( mIMEdit, TQT_SIGNAL( textChanged( const TQString& ) ),
           TQT_SIGNAL( modified() ) );
  label->setBuddy( mIMEdit );
  topLayout->addWidget( mIMEdit, 0, 1 );

  mEditButton = new TQPushButton( i18n( "Edit IM Addresses..." ), this);
  connect( mEditButton, TQT_SIGNAL( clicked() ), TQT_SLOT( edit() ) );
  topLayout->addMultiCellWidget( mEditButton, 1, 1, 0, 1 );

  topLayout->activate();
}

IMEditWidget::~IMEditWidget()
{
}

void IMEditWidget::setReadOnly( bool readOnly )
{
  mIMEdit->setReadOnly( readOnly );
  mReadOnly = readOnly;
//  mEditButton->setEnabled( !readOnly );
}
void IMEditWidget::setPreferredIM( const TQString &addr )
{
  bool blocked = mIMEdit->signalsBlocked();
  mIMEdit->blockSignals( true );
  mIMEdit->setText( addr );
  mIMEdit->blockSignals( blocked );
}
void IMEditWidget::setIMs( const TQStringList &list )
{
  mIMList = list;

  bool blocked = mIMEdit->signalsBlocked();
  mIMEdit->blockSignals( true );
  if ( list.count() > 0 )
    mIMEdit->setText( list[ 0 ] );
  else
    mIMEdit->setText( "" );
  mIMEdit->blockSignals( blocked );
}

TQStringList IMEditWidget::ims()
{
  if ( mIMEdit->text().isEmpty() ) {
    if ( mIMList.count() > 0 )
      mIMList.remove( mIMList.begin() );
  } else {
    if ( mIMList.count() > 0 )
      mIMList.remove( mIMList.begin() );

    mIMList.prepend( mIMEdit->text() );
  }

  return mIMList;
}
TQString IMEditWidget::preferredIM()
{
  return mIMEdit->text();
}
void IMEditWidget::edit()
{
  IMEditorWidget dlg(this, mIMEdit->text());
  dlg.loadContact(&mAddressee);
  dlg.setReadOnly(mReadOnly);

  if ( dlg.exec() ) {
    if ( dlg.isModified() ) {
      //Stores the changes into mAddressee.  mAddressee isn't actually saved to the addressbook
      //until we save the record.
      dlg.storeContact(&mAddressee);
      mIMEdit->setText( dlg.preferred() );
      emit modified();
    }
  }
}

void IMEditWidget::textChanged( const TQString &text )
{
  if ( mIMList.count() > 0 )
    mIMList.remove( mIMList.begin() );

  mIMList.prepend( text );
}


#include "imeditwidget.moc"

