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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qtoolbutton.h>
#include <qtooltip.h>

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

IMEditWidget::IMEditWidget( QWidget *parent, KABC::Addressee &addr, const char *name )
  : QWidget( parent, name ), mAddressee(addr)
{
  QGridLayout *topLayout = new QGridLayout( this, 2, 2, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "IM address:" ), this );
  topLayout->addWidget( label, 0, 0 );

  mIMEdit = new KLineEdit( this );
  connect( mIMEdit, SIGNAL( textChanged( const QString& ) ),
           SLOT( textChanged( const QString& ) ) );
  connect( mIMEdit, SIGNAL( textChanged( const QString& ) ),
           SIGNAL( modified() ) );
  label->setBuddy( mIMEdit );
  topLayout->addWidget( mIMEdit, 0, 1 );

  mEditButton = new QPushButton( i18n( "Edit IM Addresses..." ), this);
  connect( mEditButton, SIGNAL( clicked() ), SLOT( edit() ) );
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
void IMEditWidget::setPreferredIM( const QString &addr )
{
  bool blocked = mIMEdit->signalsBlocked();
  mIMEdit->blockSignals( true );
  mIMEdit->setText( addr );
  mIMEdit->blockSignals( blocked );
}
void IMEditWidget::setIMs( const QStringList &list )
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

QStringList IMEditWidget::ims()
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
QString IMEditWidget::preferredIM()
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

void IMEditWidget::textChanged( const QString &text )
{
  if ( mIMList.count() > 0 )
    mIMList.remove( mIMList.begin() );

  mIMList.prepend( text );
}


#include "imeditwidget.moc"

