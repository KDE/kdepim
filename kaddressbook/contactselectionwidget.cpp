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

#include <Akonadi/CollectionComboBox>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/ItemFetchJob>
#include <Akonadi/ItemFetchScope>
#include <Akonadi/RecursiveItemFetchJob>

#include <KLocale>

#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QVBoxLayout>

ContactSelectionWidget::ContactSelectionWidget( QItemSelectionModel *selectionModel,
                                                QWidget *parent )
  : QWidget( parent ), mSelectionModel( selectionModel )
{
  initGui();

  mSelectedContactsButton->setEnabled( mSelectionModel->hasSelection() );
  mAddressBookSelection->setEnabled( false );
  mAddressBookSelectionRecursive->setEnabled( false );

  connect( mAddressBookContactsButton, SIGNAL(toggled(bool)),
           mAddressBookSelection, SLOT(setEnabled(bool)) );
  connect( mAddressBookContactsButton, SIGNAL(toggled(bool)),
           mAddressBookSelectionRecursive, SLOT(setEnabled(bool)) );

  // apply default configuration
  if ( mSelectionModel->hasSelection() ) {
    mSelectedContactsButton->setChecked( true );
  } else {
    mAllContactsButton->setChecked( true );
  }
}

void ContactSelectionWidget::setMessageText( const QString &message )
{
  mMessageLabel->setText( message );
}

void ContactSelectionWidget::setDefaultAddressBook( const Akonadi::Collection &addressBook )
{
  mAddressBookSelection->setDefaultCollection( addressBook );
}

KABC::Addressee::List ContactSelectionWidget::selectedContacts() const
{
  if ( mAllContactsButton->isChecked() ) {
    return collectAllContacts();
  } else if ( mSelectedContactsButton->isChecked() ) {
    return collectSelectedContacts();
  } else if ( mAddressBookContactsButton->isChecked() ) {
    return collectAddressBookContacts();
  }

  return KABC::Addressee::List();
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
  mAddressBookSelection = new Akonadi::CollectionComboBox;
  mAddressBookSelection->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
  mAddressBookSelection->setAccessRightsFilter( Akonadi::Collection::ReadOnly );
  mAddressBookSelectionRecursive = new QCheckBox( i18n( "Include Subfolders" ) );

  group->addButton( mAllContactsButton );
  group->addButton( mSelectedContactsButton );
  group->addButton( mAddressBookContactsButton );

  boxLayout->addWidget( mAllContactsButton, 0, 0, 1, 2 );
  boxLayout->addWidget( mSelectedContactsButton, 1, 0, 1, 2 );
  boxLayout->addWidget( mAddressBookContactsButton, 2, 0, Qt::AlignTop );

  QVBoxLayout *addressBookLayout = new QVBoxLayout;
  addressBookLayout->setMargin( 0 );
  addressBookLayout->addWidget( mAddressBookSelection );
  addressBookLayout->addWidget( mAddressBookSelectionRecursive );

  boxLayout->addLayout( addressBookLayout, 2, 1 );

  layout->addWidget( groupBox );
  layout->addStretch( 1 );
}

KABC::Addressee::List ContactSelectionWidget::collectAllContacts() const
{
  Akonadi::RecursiveItemFetchJob *job =
    new Akonadi::RecursiveItemFetchJob( Akonadi::Collection::root(),
                                        QStringList() << KABC::Addressee::mimeType() );
  job->fetchScope().fetchFullPayload();

  KABC::Addressee::List contacts;
  if ( !job->exec() ) {
    return contacts;
  }

  foreach ( const Akonadi::Item &item, job->items() ) {
    if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
      contacts.append( item.payload<KABC::Addressee>() );
    }
  }

  return contacts;
}

KABC::Addressee::List ContactSelectionWidget::collectSelectedContacts() const
{
  KABC::Addressee::List contacts;

  const QModelIndexList indexes = mSelectionModel->selectedRows( 0 );
  for ( int i = 0; i < indexes.count(); ++i ) {
    const QModelIndex index = indexes.at( i );
    if ( index.isValid() ) {
      const Akonadi::Item item =
        index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
      if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
        contacts.append( item.payload<KABC::Addressee>() );
      }
    }
  }

  return contacts;
}

KABC::Addressee::List ContactSelectionWidget::collectAddressBookContacts() const
{
  KABC::Addressee::List contacts;

  const Akonadi::Collection collection = mAddressBookSelection->currentCollection();
  if ( !collection.isValid() ) {
    return contacts;
  }

  if ( mAddressBookSelectionRecursive->isChecked() ) {
    Akonadi::RecursiveItemFetchJob *job =
      new Akonadi::RecursiveItemFetchJob( collection,
                                          QStringList() << KABC::Addressee::mimeType() );
    job->fetchScope().fetchFullPayload();

    if ( !job->exec() ) {
      return contacts;
    }

    foreach ( const Akonadi::Item &item, job->items() ) {
      if ( item.hasPayload<KABC::Addressee>() ) {
        contacts.append( item.payload<KABC::Addressee>() );
      }
    }
  } else {
    Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection );
    job->fetchScope().fetchFullPayload();

    if ( !job->exec() ) {
      return contacts;
    }

    foreach ( const Akonadi::Item &item, job->items() ) {
      if ( item.hasPayload<KABC::Addressee>() ) {
        contacts.append( item.payload<KABC::Addressee>() );
      }
    }
  }

  return contacts;
}

#include "contactselectionwidget.moc"
