/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "mainview.h"
#include "contactviewitem.h"
#include "contactgroupviewitem.h"
#include "contacteditorview.h"
#include "contactgroupeditorview.h"
#include "contactlistproxy.h"

#include <akonadi/agentactionmanager.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/contact/contactsfilterproxymodel.h>
#include <akonadi/contact/standardcontactactionmanager.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/recursiveitemfetchjob.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kabc/vcardconverter.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>

#include <QtCore/QPointer>
#include <QtDeclarative/QDeclarativeEngine>

QML_DECLARE_TYPE( Akonadi::Contact::ContactViewItem )
QML_DECLARE_TYPE( Akonadi::Contact::ContactGroupViewItem )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "kaddressbook-mobile", new ContactListProxy, parent ),
    mImportProgressDialog( 0 )
{
}

void MainView::delayedInit()
{
  qmlRegisterType<Akonadi::Contact::ContactViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactView" );
  qmlRegisterType<Akonadi::Contact::ContactGroupViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactGroupView" );

  ContactImageProvider *provider = new ContactImageProvider;
  provider->setModel( itemModel() );
  engine()->addImageProvider( QLatin1String( "contact_images" ), provider );

  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "KAddressBook" ) );

  addMimeType( KABC::Addressee::mimeType() );
  addMimeType( KABC::ContactGroup::mimeType() );
  itemFetchScope().fetchFullPayload();

  KAction *action = new KAction( i18n( "New Contact" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newContact()) );
  actionCollection()->addAction( QLatin1String( "add_new_contact" ), action );
  action = new KAction( i18n( "New Contact Group" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newContactGroup()) );
  actionCollection()->addAction( QLatin1String( "add_new_contact_group" ), action );

  action = new KAction( i18n( "Import Contacts" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importVCard() ) );
  actionCollection()->addAction( QLatin1String( "import_vcards" ), action );

  action = new KAction( i18n( "Export Contacts" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportVCard() ) );
  actionCollection()->addAction( QLatin1String( "export_vcards" ), action );
}

void MainView::newContact()
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->show();
}

void MainView::newContactGroup()
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->show();
}

void MainView::editItem()
{
  const Akonadi::Item::List items = mActionManager->selectedItems();
  if ( items.isEmpty() )
    return;

  const Akonadi::Item item = items.first();

  if ( item.hasPayload<KABC::Addressee>() )
    editContact( item );
  else if ( item.hasPayload<KABC::ContactGroup>() )
    editContactGroup( item );
}

void MainView::editContact( const Akonadi::Item &item )
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->loadContact( item );
  editor->show();
}

void MainView::editContactGroup( const Akonadi::Item &item )
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );
  editor->loadContactGroup( item );
  editor->show();
}

void MainView::importVCard()
{
  QString fileName;
  KABC::Addressee::List contacts;

  const QStringList fileNames = KFileDialog::getOpenFileNames( KUrl(), "*.vcf|vCards", 0,
                                                               i18n( "Select vCard to Import" ) );

  if ( fileNames.count() == 0 )
    return;

  const QString caption( i18n( "vCard Import Failed" ) );
  bool anyFailures = false;

  KABC::VCardConverter converter;

  foreach ( const QString &fileName, fileNames ) {
    QFile file( fileName );

    if ( file.open( QIODevice::ReadOnly ) ) {
      const QByteArray data = file.readAll();
      file.close();
      if ( data.size() > 0 ) {
        contacts += converter.parseVCards( data );
      }
    } else {
      const QString msg = i18nc( "@info",
                                 "<para>When trying to read the vCard, there was an error opening the file <filename>%1</filename>:</para>"
                                 "<para>%2</para>",
                                 fileName,
                                 i18nc( "QFile", file.errorString().toLatin1() ) );
      KMessageBox::error( 0, msg, caption );
      anyFailures = true;
    }
  }

  if ( contacts.isEmpty() ) {
    if ( anyFailures && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No contacts were imported, due to errors with the vCards." ) );
    else if ( !anyFailures )
      KMessageBox::information( 0, i18n( "The vCard does not contain any contacts." ) );

    return; // nothing to import
  }

  const QStringList mimeTypes( KABC::Addressee::mimeType() );

  QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog();
  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Address Book" ) );
  dlg->setDescription( i18n( "Select the address book the imported contact(s) shall be saved in:" ) );

  // preselect the currently selected folder
  const QModelIndexList indexes = regularSelectionModel()->selectedRows();
  if ( !indexes.isEmpty() ) {
    const QModelIndex collectionIndex = indexes.first();
    const Akonadi::Collection collection = collectionIndex.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      dlg->setDefaultCollection( collection );
  }

  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    return;
  }

  const Akonadi::Collection collection = dlg->selectedCollection();
  delete dlg;

  if ( !mImportProgressDialog ) {
    mImportProgressDialog = new KProgressDialog( 0, i18n( "Import Contacts" ) );
    mImportProgressDialog->setLabelText( i18np( "Importing one contact to %2", "Importing %1 contacts to %2",
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
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotImportJobDone( KJob* ) ) );
  }
}

void MainView::slotImportJobDone( KJob* )
{
  if ( !mImportProgressDialog )
    return;

  QProgressBar *progressBar = mImportProgressDialog->progressBar();

  progressBar->setValue( progressBar->value() + 1 );

  // cleanup on last step
  if ( progressBar->value() == progressBar->maximum() ) {
    mImportProgressDialog->deleteLater();
    mImportProgressDialog = 0;
  }
}

static QString contactFileName( const KABC::Addressee &contact )
{
  if ( !contact.givenName().isEmpty() && !contact.familyName().isEmpty() )
    return QString( "%1_%2" ).arg( contact.givenName() ).arg( contact.familyName() );

  if ( !contact.familyName().isEmpty() )
    return contact.familyName();

  if ( !contact.givenName().isEmpty() )
    return contact.givenName();

  if ( !contact.organization().isEmpty() )
    return contact.organization();

  return contact.uid();
}

static bool doExport( const QString &fileName, const QByteArray &data )
{
  KUrl url( fileName );
  if ( url.isLocalFile() && QFileInfo( url.toLocalFile() ).exists() ) {
    if ( KMessageBox::questionYesNo( 0, i18n( "Do you want to overwrite file \"%1\"", url.toLocalFile() ) ) == KMessageBox::No )
      return false;
  }

  QFile file( fileName );
  if ( !file.open( QIODevice::WriteOnly ) )
    return false;

  file.write( data );
  file.close();

  return true;
}

void MainView::exportVCard()
{
  Akonadi::Collection::List selectedCollections;
  const QModelIndexList indexes = regularSelectionModel()->selectedRows();
  foreach ( const QModelIndex &index, indexes ) {
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      selectedCollections << collection;
  }

  bool exportAllContacts = false;
  if ( !selectedCollections.isEmpty() ) {
    const QString msg = i18n( "Which contacts shall be exported?" );
    switch ( KMessageBox::questionYesNo( 0, msg, QString(), KGuiItem(i18n( "All Contacts" ) ),
                                         KGuiItem( i18n( "Contacts in current folder" ) ) ) ) {
      case KMessageBox::Yes:
        exportAllContacts = true;
        break;
      case KMessageBox::No: // fall through
      default:
        exportAllContacts = false;
    }
  } else {
    exportAllContacts = true;
  }

  Akonadi::Item::List contactItems;
  if ( exportAllContacts ) {
    Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob( Akonadi::Collection::root(),
                                                                              QStringList() << KABC::Addressee::mimeType() );
    job->fetchScope().fetchFullPayload();

    job->exec();

    contactItems << job->items();
  } else {
    foreach ( const Akonadi::Collection &collection, selectedCollections ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection );
      job->fetchScope().fetchFullPayload();

      if ( job->exec() )
        contactItems << job->items();
    }
  }

  KABC::Addressee::List contacts;

  foreach ( const Akonadi::Item &item, contactItems ) {
    if ( item.hasPayload<KABC::Addressee>() )
      contacts << item.payload<KABC::Addressee>();
  }

  if ( contacts.isEmpty() )
    return;

  KABC::VCardConverter converter;
  QString fileName;

  bool ok = true;
  if ( contacts.count() == 1 ) {
    fileName = KFileDialog::getSaveFileName( contactFileName( contacts.first() ) + QLatin1String( ".vcf" ) );
    if ( fileName.isEmpty() ) // user canceled export
      return;

    ok = doExport( fileName, converter.createVCards( contacts, KABC::VCardConverter::v3_0 ) );
  } else {
    const QString msg = i18n( "You have selected a list of contacts, shall they be "
                              "exported to several files?" );

    switch ( KMessageBox::questionYesNo( parentWidget(), msg, QString(), KGuiItem(i18n( "Export to Several Files" ) ),
                                         KGuiItem( i18n( "Export to One File" ) ) ) ) {
      case KMessageBox::Yes: {
        const KUrl baseUrl = KFileDialog::getExistingDirectoryUrl();
        if ( baseUrl.isEmpty() )
          return; // user canceled export

        foreach ( const KABC::Addressee &contact, contacts ) {

          fileName = baseUrl.url() + QDir::separator() + contactFileName( contact ) + QLatin1String( ".vcf" );

          bool tmpOk = false;

          tmpOk = doExport( fileName, converter.createVCard( contact, KABC::VCardConverter::v3_0 ) );

          ok = ok && tmpOk;
        }
        break;
      }
      case KMessageBox::No:
      default: {
        fileName = KFileDialog::getSaveFileName( KUrl( "addressbook.vcf" ) );
        if ( fileName.isEmpty() )
          return; // user canceled export

        ok = doExport( fileName, converter.createVCards( contacts, KABC::VCardConverter::v3_0 ) );
      }
    }
  }

  if ( !ok )
    qDebug() << "error";
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  mActionManager = new Akonadi::StandardContactActionManager( actionCollection(), this );
  mActionManager->setCollectionSelectionModel( collectionSelectionModel );
  mActionManager->setItemSelectionModel( itemSelectionModel );

  mActionManager->createAllActions();
  mActionManager->interceptAction( Akonadi::StandardContactActionManager::CreateContact );
  mActionManager->interceptAction( Akonadi::StandardContactActionManager::CreateContactGroup );
  mActionManager->interceptAction( Akonadi::StandardContactActionManager::EditItem );
  mActionManager->interceptAction( Akonadi::StandardActionManager::CreateResource );

  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContact ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContact() ) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContactGroup() ) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::EditItem ), SIGNAL( triggered( bool ) ),
           this, SLOT( editItem() ) );
  connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  mActionManager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Synchronize contacts\nin account",
                                                                                               "Synchronize contacts\nin accounts" ) );
  mActionManager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Synchronize contacts\nin folder",
                                                                                                 "Synchronize contacts\nin folders" ) );
  mActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  mActionManager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  mActionManager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete contact", "Delete contacts" ) );
  mActionManager->action( Akonadi::StandardActionManager::MoveItemToMenu )->setText( i18n( "Move contact\nto folder" ) );
  mActionManager->action( Akonadi::StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy contact\nto folder" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize\nall contacts" ) );
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = new Akonadi::AgentActionManager( actionCollection(), this );
  manager->setSelectionModel( selectionModel );
  manager->createAllActions();

  manager->action( Akonadi::AgentActionManager::CreateAgentInstance )->setText( i18n( "Add" ) );
  manager->action( Akonadi::AgentActionManager::DeleteAgentInstance )->setText( i18n( "Delete" ) );
  manager->action( Akonadi::AgentActionManager::ConfigureAgentInstance )->setText( i18n( "Edit" ) );

  manager->interceptAction( Akonadi::AgentActionManager::CreateAgentInstance );

  connect( manager->action( Akonadi::AgentActionManager::CreateAgentInstance ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageText,
                           i18n( "Could not create account: %1" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageTitle,
                           i18n( "Account creation failed" ) );

  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxTitle,
                           i18nc( "@title:window", "Delete Account?" ) );
  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxText,
                           i18n( "Do you really want to delete the selected account?" ) );
}

QAbstractProxyModel* MainView::itemFilterModel() const
{
  return new Akonadi::ContactsFilterProxyModel();
}

#include "mainview.moc"
