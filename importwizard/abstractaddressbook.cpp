/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "abstractaddressbook.h"
#include "importwizard.h"
#include "importaddressbookpage.h"

#include <KABC/Addressee>
#include <kabc/contactgroup.h>
#include <KLocale>
#include <KDebug>
#include <AkonadiCore/ItemCreateJob>
#include <AkonadiCore/Item>
#include <AkonadiWidgets/CollectionDialog>

#include <QPointer>

AbstractAddressBook::AbstractAddressBook(ImportWizard *parent)
    : mImportWizard(parent), mCollection( -1 )
{
}

AbstractAddressBook::~AbstractAddressBook()
{

}

bool AbstractAddressBook::selectAddressBook()
{
    addAddressBookImportInfo( i18n( "Creating new contact..." ) );
    if ( !mCollection.isValid() )
    {
        const QStringList mimeTypes( KABC::Addressee::mimeType() );
        QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog( mImportWizard );
        dlg->setMimeTypeFilter( mimeTypes );
        dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
        dlg->setCaption( i18n( "Select Address Book" ) );
        dlg->setDescription( i18n( "Select the address book the new contact shall be saved in:" ) );

        if ( dlg->exec() == QDialog::Accepted && dlg ) {
            mCollection = dlg->selectedCollection();
        } else {
            addAddressBookImportError( i18n( "Address Book was not selected." ) );
            delete dlg;
            return false;
        }
        delete dlg;
    }
    return true;
}

void AbstractAddressBook::createGroup(const KABC::ContactGroup& group)
{
    if (selectAddressBook()) {
        Akonadi::Item item;
        item.setPayload<KABC::ContactGroup>( group );
        item.setMimeType( KABC::ContactGroup::mimeType() );

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, mCollection );
        connect( job, SIGNAL(result(KJob*)), SLOT(slotStoreDone(KJob*)) );
    }
}

void AbstractAddressBook::addImportNote(KABC::Addressee& address, const QString &applicationName)
{
    QString currentNote = address.note();
    if (!currentNote.isEmpty())
        currentNote += QLatin1Char('\n');
    currentNote += i18n("Imported from \"%1\"", applicationName);
    address.setNote(currentNote);
}

void AbstractAddressBook::createContact( const KABC::Addressee& address )
{
    if (selectAddressBook()) {
        Akonadi::Item item;
        item.setPayload<KABC::Addressee>( address );
        item.setMimeType( KABC::Addressee::mimeType() );
        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, mCollection );
        connect( job, SIGNAL(result(KJob*)), SLOT(slotStoreDone(KJob*)) );
    }
}

void AbstractAddressBook::slotStoreDone(KJob*job)
{
    if ( job->error() ) {
        kDebug()<<" job->errorString() : "<<job->errorString();
        addAddressBookImportError( i18n( "Error during contact creation: %1", job->errorString() ) );
        return;
    }
    addAddressBookImportInfo( i18n( "Contact creation complete" ) );
}

void AbstractAddressBook::addImportInfo( const QString& log )
{
    addAddressBookImportInfo(log);
}

void AbstractAddressBook::addImportError( const QString& log )
{
    addAddressBookImportError(log);
}

void AbstractAddressBook::addAddressBookImportInfo( const QString& log )
{
    mImportWizard->importAddressBookPage()->addImportInfo( log );
}

void AbstractAddressBook::addAddressBookImportError( const QString& log )
{
    mImportWizard->importAddressBookPage()->addImportError( log );
}

void AbstractAddressBook::cleanUp()
{
    mCollection = Akonadi::Collection();
}



