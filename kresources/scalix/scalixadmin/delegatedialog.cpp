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

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqtoolbutton.h>

#include <klocale.h>

#include "jobs.h"
#include "ldapdialog.h"

#include "delegatedialog.h"

DelegateDialog::DelegateDialog( TQWidget *parent )
  : KDialogBase( parent, "", true, "", Ok | Cancel, Ok, true )
{
  TQWidget *page = new TQWidget( this );
  TQGridLayout *layout = new TQGridLayout( page, 5, 3, 11, 6 );

  TQLabel *label = new TQLabel( i18n( "User:" ), page );
  layout->addWidget( label, 0, 0 );

  mEmail = new TQLineEdit( page );
  layout->addWidget( mEmail, 0, 1 );

  TQToolButton *emailSelector = new TQToolButton( page );
  emailSelector->setUsesTextLabel( true );
  emailSelector->setTextLabel( i18n( "..." ) );
  layout->addWidget( emailSelector, 0, 2 );

  TQValueList<Scalix::DelegateTypes> types;
  types << Scalix::SendOnBehalfOf;
  types << Scalix::SeePrivate;
  types << Scalix::GetMeetings;
  types << Scalix::InsteadOfMe;

  int row = 1;
  for ( uint i = 0; i < types.count(); ++i ) {
    TQCheckBox *box = new TQCheckBox( Scalix::Delegate::rightsAsString( types[ i ] ), page );
    layout->addMultiCellWidget( box, row, row, 1, 2 );

    mRights.insert( types[ i ], box );
    row++;
  }

  connect( emailSelector, TQT_SIGNAL( clicked() ), TQT_SLOT( selectEmail() ) );

  setMainWidget( page );
}

void DelegateDialog::setDelegate( const Scalix::Delegate &delegate )
{
  mEmail->setText( delegate.email() );

  TQMap<int, TQCheckBox*>::Iterator it;
  for ( it = mRights.begin(); it != mRights.end(); ++it )
    it.data()->setChecked( delegate.rights() & it.key() );
}

Scalix::Delegate DelegateDialog::delegate() const
{
  int rights = 0;

  TQMap<int, TQCheckBox*>::ConstIterator it;
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

  const TQString email = dlg.selectedUser();
  if ( email.isEmpty() )
    return;

  mEmail->setText( email );
}

#include "delegatedialog.moc"
