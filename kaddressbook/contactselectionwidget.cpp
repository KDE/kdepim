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

#include "contactselectionwidget.h"

#include "recursiveitemfetchjob.h"

#include <akonadi/contact/addressbookcombobox.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>
#include <klocale.h>

#include <QtCore/QAbstractItemModel>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

ContactSelectionWidget::ContactSelectionWidget( QAbstractItemModel *model, QItemSelectionModel *selectionModel, QWidget *parent )
  : QWidget( parent ), mModel( model ), mSelectionModel( selectionModel )
{
  initGui();

  mSelectedContactsButton->setEnabled( mSelectionModel->hasSelection() );
  mAddressBookSelection->setEnabled( false );

  connect( mAddressBookContactsButton, SIGNAL( toggled( bool ) ),
           mAddressBookSelection, SLOT( setEnabled( bool ) ) );

  // apply default configuration
  if ( mSelectionModel->hasSelection() )
    mSelectedContactsButton->setChecked( true );
  else
    mAllContactsButton->setChecked( true );
}

void ContactSelectionWidget::setMessageText( const QString &message )
{
  mMessageLabel->setText( message );
}

void ContactSelectionWidget::requestSelectedContacts()
{
  if ( mAllContactsButton->isChecked() )
    collectAllContacts();
  else if ( mSelectedContactsButton->isChecked() )
    collectSelectedContacts();
  else if ( mAddressBookContactsButton->isChecked() )
    collectAddressBookContacts();
}

void ContactSelectionWidget::initGui()
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  mMessageLabel = new QLabel;
  layout->addWidget( mMessageLabel );

  QButtonGroup *group = new QButtonGroup( this );

  QGroupBox *groupBox = new QGroupBox;

  QGridLayout *boxLayout = new QGridLayout;
  groupBox->setLayout( boxLayout );

  mAllContactsButton = new QRadioButton( i18n( "All contacts" ) );
  mSelectedContactsButton = new QRadioButton( i18n( "Selected contacts" ) );
  mAddressBookContactsButton = new QRadioButton( i18n( "All contacts from:" ) );
  mAddressBookSelection = new Akonadi::AddressBookComboBox( Akonadi::AddressBookComboBox::ContactsOnly,
                                                            Akonadi::AddressBookComboBox::Readable );

  group->addButton( mAllContactsButton );
  group->addButton( mSelectedContactsButton );
  group->addButton( mAddressBookContactsButton );

  boxLayout->addWidget( mAllContactsButton, 0, 0, 1, 2 );
  boxLayout->addWidget( mSelectedContactsButton, 1, 0, 1, 2 );
  boxLayout->addWidget( mAddressBookContactsButton, 2, 0 );
  boxLayout->addWidget( mAddressBookSelection, 2, 1 );

  layout->addWidget( groupBox );
  layout->addStretch( 1 );
}

void ContactSelectionWidget::collectAllContacts()
{
  KABC::Addressee::List contacts;

  for ( int i = 0; i < mModel->rowCount(); ++i ) {
    const QModelIndex index = mModel->index( i, 0 );
    if ( index.isValid() ) {
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KABC::Addressee>() )
        contacts.append( item.payload<KABC::Addressee>() );
    }
  }

  emit selectedContacts( contacts );
}

void ContactSelectionWidget::collectSelectedContacts()
{
  KABC::Addressee::List contacts;

  const QModelIndexList indexes = mSelectionModel->selectedRows( 0 );
  for ( int i = 0; i < indexes.count(); ++i ) {
    const QModelIndex index = indexes.at( i );
    if ( index.isValid() ) {
      const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KABC::Addressee>() )
        contacts.append( item.payload<KABC::Addressee>() );
    }
  }

  emit selectedContacts( contacts );
}

void ContactSelectionWidget::collectAddressBookContacts()
{
  const Akonadi::Collection collection = mAddressBookSelection->selectedAddressBook();
  if ( !collection.isValid() ) {
    emit selectedContacts( KABC::Addressee::List() );
    return;
  }

  Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob( collection, this );
  job->fetchScope().fetchFullPayload();

  connect( job, SIGNAL( result( KJob* ) ), SLOT( addressBookContactsFetched( KJob* ) ) );
  job->start();
}

void ContactSelectionWidget::addressBookContactsFetched( KJob *job )
{
  KABC::Addressee::List contacts;

  if ( !job->error() ) {
    Akonadi::RecursiveItemFetchJob *fetchJob = qobject_cast<Akonadi::RecursiveItemFetchJob*>( job );
    const Akonadi::Item::List items = fetchJob->items();

    foreach ( const Akonadi::Item &item, items ) {
      if ( item.hasPayload<KABC::Addressee>() ) {
        contacts.append( item.payload<KABC::Addressee>() );
      }
    }
  }

  emit selectedContacts( contacts );
}

#include "contactselectionwidget.moc"
