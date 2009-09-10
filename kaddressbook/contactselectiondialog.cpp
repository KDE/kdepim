/*
    Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "contactselectiondialog.h"

#include "contactselectionwidget.h"

#include <klocale.h>

ContactSelectionDialog::ContactSelectionDialog( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent )
  : KDialog( parent )
{
  setCaption( i18n( "Select Contacts" ) );
  setButtons( Ok | Cancel );

  mSelectionWidget = new ContactSelectionWidget( model, selectionModel, this );
  setMainWidget( mSelectionWidget );

  connect( mSelectionWidget, SIGNAL( selectedContacts( const KABC::Addressee::List& ) ),
           this, SLOT( slotSelectedContacts( const KABC::Addressee::List& ) ) );
  connect( this, SIGNAL( okClicked() ), this, SLOT( slotOkClicked() ) );

  setInitialSize( QSize( 450, 220 ) );
}

void ContactSelectionDialog::setMessageText( const QString &message )
{
  mSelectionWidget->setMessageText( message );
}

KABC::Addressee::List ContactSelectionDialog::selectedContacts() const
{
  return mContacts;
}

void ContactSelectionDialog::slotSelectedContacts( const KABC::Addressee::List &contacts )
{
  mContacts = contacts;
  accept();
}

void ContactSelectionDialog::slotButtonClicked( int button )
{
  if ( button == KDialog::Ok ) {
    // this will trigger slotSelectedContacts via signal/slot
    mSelectionWidget->requestSelectedContacts();
  } else {
    KDialog::slotButtonClicked( button );
  }
}

#include "contactselectiondialog.moc"
