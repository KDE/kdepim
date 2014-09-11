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
#include "utils.h"

#include <AkonadiWidgets/CollectionComboBox>
#include <AkonadiCore/EntityTreeModel>
#include <AkonadiCore/ItemFetchJob>
#include <AkonadiCore/ItemFetchScope>
#include <AkonadiCore/RecursiveItemFetchJob>

#include <KLocalizedString>

#include <QButtonGroup>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

ContactSelectionWidget::ContactSelectionWidget( QItemSelectionModel *selectionModel,
                                                QWidget *parent )
    : QWidget( parent ), mSelectionModel( selectionModel )
{
    initGui();

    mSelectedContactsButton->setEnabled( mSelectionModel->hasSelection() );
    mAddressBookSelection->setEnabled( false );
    mAddressBookSelectionRecursive->setEnabled( false );

    connect(mAddressBookContactsButton, &QRadioButton::toggled, mAddressBookSelection, &Akonadi::CollectionComboBox::setEnabled);
    connect(mAddressBookContactsButton, &QRadioButton::toggled, mAddressBookSelectionRecursive, &QCheckBox::setEnabled);

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

Akonadi::Item::List ContactSelectionWidget::selectedContactsItem() const
{
    if ( mAllContactsButton->isChecked() ) {
        return collectAllContactsItem();
    } else if ( mSelectedContactsButton->isChecked() ) {
        return collectSelectedContactsItem();
    } else if ( mAddressBookContactsButton->isChecked() ) {
        return collectAddressBookContactsItem();
    }

    return Akonadi::Item::List();
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

    mAllContactsButton = new QRadioButton( i18nc( "@option:radio", "All contacts" ) );
    mAllContactsButton->setToolTip(
                i18nc( "@info:tooltip", "All contacts from all your address books" ) );
    mAllContactsButton->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Choose this option you want to select all your contacts from "
                       "all your address books." ) );

    mSelectedContactsButton = new QRadioButton( i18nc( "@option:radio","Selected contacts" ) );
    mSelectedContactsButton->setToolTip(
                i18nc( "@info:tooltip", "Only the contacts currently selected" ) );
    mSelectedContactsButton->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Choose this option if you want only the contacts you have already "
                       "selected in the graphical interface." ) );

    mAddressBookContactsButton = new QRadioButton( i18nc( "@option:radio", "All contacts from:" ) );
    mAddressBookContactsButton->setToolTip(
                i18nc( "@info:tooltip", "All contacts from a chosen address book" ) );
    mAddressBookContactsButton->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Choose this option if you want to select all the contacts from only one "
                       "of your address books.  Once this option is clicked you will be provided "
                       "a drop down box listing all those address books and permitted to select "
                       "the one you want." ) );

    mAddressBookSelection = new Akonadi::CollectionComboBox;
    mAddressBookSelection->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType() );
    mAddressBookSelection->setAccessRightsFilter( Akonadi::Collection::ReadOnly );
    mAddressBookSelection->setExcludeVirtualCollections( true );
    mAddressBookSelectionRecursive = new QCheckBox( i18nc( "@option:check", "Include Subfolders" ) );
    mAddressBookSelectionRecursive->setToolTip(
                i18nc( "@info:tooltip", "Select all subfolders including the top-level folder" ) );
    mAddressBookSelectionRecursive->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "Check this box if you want to select all contacts from this folder, "
                       "including all subfolders.  If you only want the contacts from the "
                       "top-level folder then leave this box unchecked." ) );

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

Akonadi::Item::List ContactSelectionWidget::collectAllContactsItem() const
{
    Akonadi::RecursiveItemFetchJob *job =
            new Akonadi::RecursiveItemFetchJob( Akonadi::Collection::root(),
                                                QStringList() << KABC::Addressee::mimeType() );
    job->fetchScope().fetchFullPayload();

    Akonadi::Item::List lst;
    if ( !job->exec() ) {
        return lst;
    }

    foreach ( const Akonadi::Item &item, job->items() ) {
        if ( item.isValid() && item.hasPayload<KABC::Addressee>() ) {
            lst.append( item );
        }
    }

    return lst;
}

Akonadi::Item::List ContactSelectionWidget::collectSelectedContactsItem() const
{
    Akonadi::Item::List lst = Utils::collectSelectedContactsItem(mSelectionModel);

    return lst;
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


Akonadi::Item::List ContactSelectionWidget::collectAddressBookContactsItem() const
{
    Akonadi::Item::List lst;

    const Akonadi::Collection collection = mAddressBookSelection->currentCollection();
    if ( !collection.isValid() ) {
        return lst;
    }

    if ( mAddressBookSelectionRecursive->isChecked() ) {
        Akonadi::RecursiveItemFetchJob *job =
                new Akonadi::RecursiveItemFetchJob( collection,
                                                    QStringList() << KABC::Addressee::mimeType() );
        job->fetchScope().fetchFullPayload();

        if ( !job->exec() ) {
            return lst;
        }

        foreach ( const Akonadi::Item &item, job->items() ) {
            if ( item.hasPayload<KABC::Addressee>() ) {
                lst.append( item );
            }
        }
    } else {
        Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection );
        job->fetchScope().fetchFullPayload();

        if ( !job->exec() ) {
            return lst;
        }

        foreach ( const Akonadi::Item &item, job->items() ) {
            if ( item.hasPayload<KABC::Addressee>() ) {
                lst.append( item );
            }
        }
    }

    return lst;
}
