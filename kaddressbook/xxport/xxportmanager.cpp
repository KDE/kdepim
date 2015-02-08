/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "xxportmanager.h"
#include "contactselectiondialog.h"

#include <Akonadi/Collection>
#include <Akonadi/CollectionDialog>
#include <Akonadi/EntityTreeModel>
#include <Akonadi/Item>
#include <Akonadi/ItemCreateJob>

#include <KLocalizedString>
#include <KMessageBox>
#include <KProgressDialog>

#include <QtCore/QPointer>
#include <QtCore/QSignalMapper>
#include <QAction>
#include <QItemSelectionModel>
#include <QWidget>

XXPortManager::XXPortManager( QWidget *parent )
    : QObject( parent ), mSelectionModel( 0 ),
      mParentWidget( parent ), mImportProgressDialog( 0 )
{
    mImportMapper = new QSignalMapper( this );
    mExportMapper = new QSignalMapper( this );

    connect( mImportMapper, SIGNAL(mapped(QString)),
             this, SLOT(slotImport(QString)) );
    connect( mExportMapper, SIGNAL(mapped(QString)),
             this, SLOT(slotExport(QString)) );
}

XXPortManager::~XXPortManager()
{
}

void XXPortManager::addImportAction( QAction *action, const QString &identifier )
{
    mImportMapper->setMapping( action, identifier );
    connect( action, SIGNAL(triggered(bool)), mImportMapper, SLOT(map()) );
}

void XXPortManager::addExportAction( QAction *action, const QString &identifier )
{
    mExportMapper->setMapping( action, identifier );
    connect( action, SIGNAL(triggered(bool)), mExportMapper, SLOT(map()) );
}

void XXPortManager::setSelectionModel( QItemSelectionModel *selectionModel )
{
    mSelectionModel = selectionModel;
}

void XXPortManager::setDefaultAddressBook( const Akonadi::Collection &addressBook )
{
    mDefaultAddressBook = addressBook;
}

void XXPortManager::importFile( const KUrl &url)
{
    QString identifier;
    if (url.path().endsWith(QLatin1String("vcf"))) {
        identifier = QLatin1String("vcard30");
    } else if (url.path().endsWith(QLatin1String("ldif"))) {
        identifier = QLatin1String("ldif");
    } else if (url.path().endsWith(QLatin1String("gmx"))) {
        identifier = QLatin1String("gmx");
    }
    if (identifier.isEmpty())
        return;
    XXPort *xxport = mFactory.createXXPort( identifier, mParentWidget );
    if( !xxport ) {
        return;
    }
    xxport->setOption(QLatin1String("importUrl"), url.path());
    const KABC::Addressee::List contacts = xxport->importContacts();

    delete xxport;
    import(contacts);
}

void XXPortManager::slotImport( const QString &identifier )
{
    const XXPort *xxport = mFactory.createXXPort( identifier, mParentWidget );
    if( !xxport ) {
        return;
    }

    const KABC::Addressee::List contacts = xxport->importContacts();

    delete xxport;
    import(contacts);
}

void XXPortManager::import(const KABC::Addressee::List &contacts)
{
    if ( contacts.isEmpty() ) { // nothing to import
        return;
    }

    const QStringList mimeTypes( KABC::Addressee::mimeType() );

    QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog( mParentWidget );
    dlg->setMimeTypeFilter( mimeTypes );
    dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
    dlg->setCaption( i18n( "Select Address Book" ) );
    dlg->setDescription(
                i18n( "Select the address book the imported contact(s) shall be saved in:" ) );
    dlg->setDefaultCollection( mDefaultAddressBook );

    if ( !dlg->exec() || !dlg ) {
        delete dlg;
        return;
    }

    const Akonadi::Collection collection = dlg->selectedCollection();
    delete dlg;

    if ( !mImportProgressDialog ) {
        mImportProgressDialog = new KProgressDialog( mParentWidget, i18n( "Import Contacts" ) );
        mImportProgressDialog->setLabelText(
                    i18np( "Importing one contact to %2", "Importing %1 contacts to %2",
                           contacts.count(), collection.name() ) );
        mImportProgressDialog->setAllowCancel( false );
        mImportProgressDialog->setAutoClose( true );
        mImportProgressDialog->progressBar()->setRange( 1, contacts.count() );
    }

    mImportProgressDialog->show();

    for ( int i = 0; i < contacts.count(); ++i ) {
        Akonadi::Item item;
        item.setPayload<KABC::Addressee>( contacts.at( i ) );
        item.setMimeType( KABC::Addressee::mimeType() );

        Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
        connect( job, SIGNAL(result(KJob*)), SLOT(slotImportJobDone(KJob*)) );
    }
}

void XXPortManager::slotImportJobDone( KJob * )
{
    if ( !mImportProgressDialog ) {
        return;
    }

    QProgressBar *progressBar = mImportProgressDialog->progressBar();

    progressBar->setValue( progressBar->value() + 1 );

    // cleanup on last step
    if ( progressBar->value() == progressBar->maximum() ) {
        mImportProgressDialog->deleteLater();
        mImportProgressDialog = 0;
    }
}

void XXPortManager::slotExport( const QString &identifier )
{
    if ( !mSelectionModel ) {
        return;
    }

    QPointer<ContactSelectionDialog> dlg =
            new ContactSelectionDialog( mSelectionModel, mParentWidget );
    dlg->setMessageText( i18n( "Which contact do you want to export?" ) );
    dlg->setDefaultAddressBook( mDefaultAddressBook );
    if ( !dlg->exec() || !dlg ) {
        delete dlg;
        return;
    }

    const KABC::AddresseeList contacts = dlg->selectedContacts();
    delete dlg;

    if ( contacts.isEmpty() ) {
        KMessageBox::sorry( 0, i18n( "You have not selected any contacts to export." ) );
        return;
    }

    const XXPort *xxport = mFactory.createXXPort( identifier, mParentWidget );
    if ( !xxport ) {
        return;
    }

    xxport->exportContacts( contacts );

    delete xxport;
}

