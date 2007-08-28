/*
 *   This file is part of ScalixAdmin.
 *
 *   Copyright (C) 2007 Trolltech ASA. All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qtoolbutton.h>

#include <klocale.h>

#include "jobs.h"
#include "ldapdialog.h"

#include "delegatedialog.h"

DelegateDialog::DelegateDialog( QWidget *parent )
  : KDialogBase( parent, "", true, "", Ok | Cancel, Ok, true )
{
  QWidget *page = new QWidget( this );
  QGridLayout *layout = new QGridLayout( page, 5, 3, 11, 6 );

  QLabel *label = new QLabel( i18n( "User:" ), page );
  layout->addWidget( label, 0, 0 );

  mEmail = new QLineEdit( page );
  layout->addWidget( mEmail, 0, 1 );

  QToolButton *emailSelector = new QToolButton( page );
  emailSelector->setUsesTextLabel( true );
  emailSelector->setTextLabel( i18n( "..." ) );
  layout->addWidget( emailSelector, 0, 2 );

  QValueList<Scalix::DelegateTypes> types;
  types << Scalix::SendOnBehalfOf;
  types << Scalix::SeePrivate;
  types << Scalix::GetMeetings;
  types << Scalix::InsteadOfMe;

  int row = 1;
  for ( uint i = 0; i < types.count(); ++i ) {
    QCheckBox *box = new QCheckBox( Scalix::Delegate::rightsAsString( types[ i ] ), page );
    layout->addMultiCellWidget( box, row, row, 1, 2 );

    mRights.insert( types[ i ], box );
    row++;
  }

  connect( emailSelector, SIGNAL( clicked() ), SLOT( selectEmail() ) );

  setMainWidget( page );
}

void DelegateDialog::setDelegate( const Scalix::Delegate &delegate )
{
  mEmail->setText( delegate.email() );

  QMap<int, QCheckBox*>::Iterator it;
  for ( it = mRights.begin(); it != mRights.end(); ++it )
    it.data()->setChecked( delegate.rights() & it.key() );
}

Scalix::Delegate DelegateDialog::delegate() const
{
  int rights = 0;

  QMap<int, QCheckBox*>::ConstIterator it;
  for ( it = mRights.begin(); it != mRights.end(); ++it )
    if ( it.data()->isChecked() )
      rights |= it.key();

  return Scalix::Delegate( mEmail->text(), rights );
}

void DelegateDialog::selectEmail()
{
  LdapDialog dlg( this );
  if ( !dlg.exec() )
    return;

  const QString email = dlg.selectedUser();
  if ( email.isEmpty() )
    return;

  mEmail->setText( email );
}

#include "delegatedialog.moc"
