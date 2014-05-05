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

#include "actionhelper.h"
#include "configwidget.h"
#include "contactviewitem.h"
#include "contactgroupviewitem.h"
#include "contacteditorview.h"
#include "contactgroupeditorview.h"
#include "contactlistproxy.h"
#include "contactsexporthandler.h"
#include "contactsguistatemanager.h"
#include "contactsimporthandler.h"
#include "searchwidget.h"

#include "libkdepim/ldap/ldapsearchdialog.h"

#include <AkonadiWidgets/agentactionmanager.h>
#include <AkonadiWidgets/collectiondialog.h>
#include <Akonadi/Contact/contactgroupexpandjob.h>
#include <Akonadi/Contact/contactsfilterproxymodel.h>
#include <Akonadi/Contact/standardcontactactionmanager.h>
#include <AkonadiCore/itemcreatejob.h>
#include <AkonadiCore/itemfetchscope.h>
#include <calendarsupport/categoryconfig.h>
#include <incidenceeditor-ng/categoryeditdialog.h>
#include <incidenceeditor-ng/editorconfig.h>
#include <KABC/kabc/addressee.h>
#include <KABC/kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kconfigskeleton.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>

#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusServiceWatcher>
#include <QDeclarativeEngine>

QML_DECLARE_TYPE( Akonadi::Contact::ContactViewItem )
QML_DECLARE_TYPE( Akonadi::Contact::ContactGroupViewItem )
QML_DECLARE_TYPE( ContactsGuiStateManager )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( QLatin1String("kaddressbook-mobile"), new ContactListProxy, parent ),
  mLdapSearchDialog( 0 )
{
}

void MainView::doDelayedInit()
{
  qmlRegisterType<Akonadi::Contact::ContactViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactView" );
  qmlRegisterType<Akonadi::Contact::ContactGroupViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactGroupView" );
  qmlRegisterUncreatableType<ContactsGuiStateManager>( "org.kde.akonadi.contacts", 4, 5, "ContactsGuiStateManager", QLatin1String( "This type is only exported for its enums" ) );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.contacts", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.contacts", 4, 5, "SearchWidget" );

  ContactImageProvider *provider = new ContactImageProvider;
  provider->setModel( itemModel() );
  engine()->addImageProvider( QLatin1String( "contact_images" ), provider );

  setWindowTitle( i18n( "Contacts" ) );

  addMimeType( KABC::Addressee::mimeType() );
  addMimeType( KABC::ContactGroup::mimeType() );
  itemFetchScope().fetchFullPayload();

  KAction *action = new KAction( i18n( "Import Contacts" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(importItems()) );
  actionCollection()->addAction( QLatin1String( "import_vcards" ), action );

  action = new KAction( i18n( "Export Contacts From This Account" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_account_vcards" ), action );

  action = new KAction( i18n( "Export Displayed Contacts" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_selected_vcards" ), action );

  action = new KAction( i18n( "Export Contact" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportSingleItem()) );
  actionCollection()->addAction( QLatin1String( "export_single_contact_vcard" ), action );

  action = new KAction( i18n( "Send mail to" ), this );
  action->setEnabled( false );
  connect( action, SIGNAL(triggered(bool)), SLOT(sendMailTo()) );
  actionCollection()->addAction( QLatin1String( "send_mail_to" ), action );

  action = new KAction( i18n( "Search in LDAP directory" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(searchLdap()) );
  actionCollection()->addAction( QLatin1String( "search_ldap" ), action );

  action = new KAction( i18n( "Configure Categories" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(configureCategories()) );
  actionCollection()->addAction( QLatin1String( "configure_categories" ), action );

  connect( itemSelectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(itemSelectionChanged(QItemSelection,QItemSelection)) );
  connect( itemActionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this, SLOT(bulkActionSelectionChanged()) );
}

void MainView::itemSelectionChanged( const QItemSelection &selected, const QItemSelection& )
{
  if ( selected.indexes().isEmpty() )
    return;

  const QModelIndex index = selected.indexes().first();
  if ( !index.isValid() )
    return;

  const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  if ( !qobject_cast<ContactsGuiStateManager*>( guiStateManager() )->inViewContactState() &&
       !qobject_cast<ContactsGuiStateManager*>( guiStateManager() )->inViewContactGroupState() ) {
    if ( item.hasPayload<KABC::Addressee>() )
      guiStateManager()->pushState( ContactsGuiStateManager::ViewContactState );
    else if ( item.hasPayload<KABC::ContactGroup>() )
      guiStateManager()->pushState( ContactsGuiStateManager::ViewContactGroupState );
  }
}

void MainView::bulkActionSelectionChanged()
{
  const bool itemsChecked = !itemActionModel()->selectedIndexes().isEmpty();

  actionCollection()->action( QLatin1String( "send_mail_to" ) )->setEnabled( itemsChecked );
}

void MainView::sendMailTo()
{
  const QModelIndexList indexes = itemActionModel()->selectedIndexes();
  if ( indexes.isEmpty() )
    return;

  // select email addresses of all checked items
  QStringList emailAddresses;
  foreach ( const QModelIndex &index, indexes ) {
    const Akonadi::Item item = index.data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    if ( !item.isValid() )
      continue;

    if ( item.hasPayload<KABC::Addressee>() ) {
      const KABC::Addressee contact = item.payload<KABC::Addressee>();
      if ( !contact.preferredEmail().isEmpty() )
        emailAddresses << contact.preferredEmail();
    }

    if ( item.hasPayload<KABC::ContactGroup>() ) {
      // resolve the contact group right now
      Akonadi::ContactGroupExpandJob *job = new Akonadi::ContactGroupExpandJob( item.payload<KABC::ContactGroup>() );
      if ( job->exec() ) {
        foreach ( const KABC::Addressee &contact, job->contacts() ) {
          if ( !contact.preferredEmail().isEmpty() )
            emailAddresses << contact.preferredEmail();
        }
      }
    }
  }

  // try to open the email composer in kmail-mobile
  QDBusInterface *interface = new QDBusInterface( QLatin1String("org.kde.kmailmobile.composer"), QLatin1String("/composer") );
  if ( !interface->isValid() ) {
    delete interface;

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher( QLatin1String("org.kde.kmailmobile.composer"), QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForRegistration, this );
    QEventLoop loop;
    connect( watcher, SIGNAL(serviceRegistered(QString)), &loop, SLOT(quit()) );
    QProcess::startDetached( QLatin1String("kmail-mobile") );
    loop.exec();

    delete watcher;

    interface = new QDBusInterface( QLatin1String("org.kde.kmailmobile.composer"),QLatin1String( "/composer") );
  }

  interface->call( QLatin1String("openComposer"), emailAddresses.join( QLatin1String(", ") ), QString(), QString(), QString(), QString() );

  delete interface;
}

void MainView::finishEdit( QObject *editor )
{
  mOpenItemEditors.remove( editor );
}

void MainView::newContact()
{
  ContactEditorView *editor = new ContactEditorView;
  connect( editor, SIGNAL(requestLaunchAccountWizard()), SLOT(launchAccountWizard()) );

  if ( regularSelectionModel()->hasSelection() ) {
    const QModelIndex index = regularSelectionModel()->selectedIndexes().first();
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      editor->setDefaultCollection( collection );
  }

  editor->show();
}

void MainView::newContactGroup()
{
  ContactGroupEditorView *editor = new ContactGroupEditorView;
  connect( editor, SIGNAL(requestLaunchAccountWizard()), SLOT(launchAccountWizard()) );

  if ( regularSelectionModel()->hasSelection() ) {
    const QModelIndex index = regularSelectionModel()->selectedIndexes().first();
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      editor->setDefaultCollection( collection );
  }

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
  if ( mOpenItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  ContactEditorView *editor = new ContactEditorView;
  editor->loadContact( item );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL(destroyed(QObject*)), SLOT(finishEdit(QObject*)) );
  connect( editor, SIGNAL(requestLaunchAccountWizard()), SLOT(launchAccountWizard()) );

  editor->show();
}

void MainView::editContactGroup( const Akonadi::Item &item )
{
  if ( mOpenItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  ContactGroupEditorView *editor = new ContactGroupEditorView;
  editor->loadContactGroup( item );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL(destroyed(QObject*)), SLOT(finishEdit(QObject*)) );
  connect( editor, SIGNAL(requestLaunchAccountWizard()), SLOT(launchAccountWizard()) );

  editor->show();
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

  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContact ), SIGNAL(triggered(bool)),
           this, SLOT(newContact()) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup ), SIGNAL(triggered(bool)),
           this, SLOT(newContactGroup()) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::EditItem ), SIGNAL(triggered(bool)),
           this, SLOT(editItem()) );
  connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL(triggered(bool)),
           this, SLOT(launchAccountWizard()) );
  connect( mActionManager, SIGNAL(actionStateUpdated()), SLOT(updateActionTexts()) );

  ActionHelper::adaptStandardActionTexts( mActionManager );

  mActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Address Book" ) );
  mActionManager->action( StandardActionManager::CreateCollection )->setProperty( "ContentMimeTypes", QStringList( KABC::Addressee::mimeType() ) );
  mActionManager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize This Address Book", "Synchronize These Address Books" ) );
  mActionManager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Address Book Properties" ) );
  mActionManager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete Address Book", "Delete Address Books" ) );
  mActionManager->action( StandardActionManager::MoveCollectionToDialog )->setText( i18n( "Move Address Book To" ) );
  mActionManager->action( StandardActionManager::CopyCollectionToDialog )->setText( i18n( "Copy Address Book To" ) );

  mActionManager->action( Akonadi::StandardContactActionManager::CreateContact )->setText( i18n( "New Contact" ) );
  mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup )->setText( i18n( "New Group Of Contacts" ) );
  mActionManager->action( Akonadi::StandardContactActionManager::EditItem )->setText( i18n( "Edit Contact" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete Contact", "Delete Contacts" ) );
  mActionManager->action( Akonadi::StandardActionManager::MoveItemToDialog )->setText( i18n( "Move Contact To" ) );
  mActionManager->action( Akonadi::StandardActionManager::CopyItemToDialog )->setText( i18n( "Copy Contact To" ) );

  actionCollection()->action( QLatin1String("synchronize_all_items") )->setText( i18n( "Synchronize All Accounts" ) );
}

void MainView::updateActionTexts()
{
  const Akonadi::Item::List items = mActionManager->selectedItems();
  if ( items.count() < 1 )
    return;

  const int itemCount = items.count();
  const Akonadi::Item item = items.first();
  const QString mimeType = item.mimeType();
  if ( mimeType == KABC::Addressee::mimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Contact", "Copy %1 Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Contact To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Contact", "Delete %1 Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Contact To" ) );
    actionCollection()->action( QLatin1String("akonadi_contact_item_edit") )->setText( i18n( "Edit Contact" ) );
  } else if ( mimeType == KABC::ContactGroup::mimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Group Of Contacts", "Copy %1 Groups Of Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Group Of Contacts To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Group Of Contacts", "Delete %1 Groups Of Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Group Of Contacts To" ) );
    actionCollection()->action( QLatin1String("akonadi_contact_item_edit") )->setText( i18n( "Edit Group Of Contacts" ) );
  }
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = createAgentActionManager( selectionModel );

  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageText,
                           ki18n( "Could not create account: %1" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageTitle,
                           i18n( "Account creation failed" ) );

  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxTitle,
                           i18nc( "@title:window", "Delete Account?" ) );
  manager->setContextText( Akonadi::AgentActionManager::DeleteAgentInstance, Akonadi::AgentActionManager::MessageBoxText,
                           i18n( "Do you really want to delete the selected account?" ) );
}

QAbstractProxyModel* MainView::createItemFilterModel() const
{
  return new Akonadi::ContactsFilterProxyModel();
}

ImportHandlerBase* MainView::importHandler() const
{
  return new ContactsImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new ContactsExportHandler();
}

GuiStateManager* MainView::createGuiStateManager() const
{
  return new ContactsGuiStateManager();
}

void MainView::searchLdap()
{
  if ( !mLdapSearchDialog ) {
    mLdapSearchDialog = new KLDAP::LdapSearchDialog( this );
    connect( mLdapSearchDialog, SIGNAL(contactsAdded()), SLOT(importFromLdap()) );
  }
  mLdapSearchDialog->show();
}

void MainView::importFromLdap()
{
  Q_ASSERT( mLdapSearchDialog );
  const KABC::Addressee::List contacts = mLdapSearchDialog->selectedContacts();
  if ( contacts.isEmpty() ) // nothing to import
    return;

  const QStringList mimeTypes( KABC::Addressee::mimeType() );

  QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog( entityTreeModel(), this );
  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Address Book" ) );
  dlg->setDescription( i18n( "Select the address book the imported contact(s) shall be saved in:" ) );

  if ( regularSelectionModel()->hasSelection() ) {
    const QModelIndex index = regularSelectionModel()->selectedIndexes().first();
    const Akonadi::Collection defaultCollection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( defaultCollection.isValid() )
      dlg->setDefaultCollection( defaultCollection );
  }

  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    return;
  }

  const Akonadi::Collection collection = dlg->selectedCollection();
  delete dlg;
  if ( !collection.isValid() )
    return;

  foreach ( const KABC::Addressee &addr, contacts ) {
    Akonadi::Item item;
    item.setPayload<KABC::Addressee>( addr );
    item.setMimeType( KABC::Addressee::mimeType() );

    new Akonadi::ItemCreateJob( item, collection );
  }
}

void MainView::configureCategories()
{
  CalendarSupport::CategoryConfig config( IncidenceEditorNG::EditorConfig::instance()->config(), 0 );
  IncidenceEditorNG::CategoryEditDialog dialog( &config, 0 );
  if ( dialog.exec() )
    config.writeConfig();
}

