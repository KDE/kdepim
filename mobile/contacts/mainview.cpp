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

#include <akonadi/agentactionmanager.h>
#include <akonadi/contact/contactgroupexpandjob.h>
#include <akonadi/contact/contactsfilterproxymodel.h>
#include <akonadi/contact/standardcontactactionmanager.h>
#include <akonadi/itemfetchscope.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <klineedit.h>
#include <klocale.h>

#include <QtCore/QPointer>
#include <QtCore/QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusServiceWatcher>
#include <QtDeclarative/QDeclarativeEngine>

QML_DECLARE_TYPE( Akonadi::Contact::ContactViewItem )
QML_DECLARE_TYPE( Akonadi::Contact::ContactGroupViewItem )
QML_DECLARE_TYPE( ContactsGuiStateManager )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "kaddressbook-mobile", new ContactListProxy, parent )
{
}

void MainView::delayedInit()
{
  qmlRegisterType<Akonadi::Contact::ContactViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactView" );
  qmlRegisterType<Akonadi::Contact::ContactGroupViewItem>( "org.kde.akonadi.contacts", 4, 5, "ContactGroupView" );
  qmlRegisterUncreatableType<ContactsGuiStateManager>( "org.kde.akonadi.contacts", 4, 5, "ContactsGuiStateManager", QLatin1String( "This type is only exported for its enums" ) );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.contacts", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.contacts", 4, 5, "SearchWidget" );

  ContactImageProvider *provider = new ContactImageProvider;
  provider->setModel( itemModel() );
  engine()->addImageProvider( QLatin1String( "contact_images" ), provider );

  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "Contacts" ) );

  addMimeType( KABC::Addressee::mimeType() );
  addMimeType( KABC::ContactGroup::mimeType() );
  itemFetchScope().fetchFullPayload();

  KAction *action = new KAction( i18n( "Import Contacts" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  actionCollection()->addAction( QLatin1String( "import_vcards" ), action );

  action = new KAction( i18n( "Export Contacts From This Account" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_account_vcards" ), action );

  action = new KAction( i18n( "Export Displayed Contacts" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_selected_vcards" ), action );

  action = new KAction( i18n( "Send mail to" ), this );
  action->setEnabled( false );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendMailTo() ) );
  actionCollection()->addAction( QLatin1String( "send_mail_to" ), action );

  connect( itemSelectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( itemSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );
  connect( itemActionModel(), SIGNAL( selectionChanged( QItemSelection, QItemSelection ) ),
           this, SLOT( bulkActionSelectionChanged() ) );
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
  QDBusInterface *interface = new QDBusInterface( "org.kde.kmailmobile.composer", "/composer" );
  if ( !interface->isValid() ) {
    delete interface;

    QDBusServiceWatcher *watcher = new QDBusServiceWatcher( "org.kde.kmailmobile.composer", QDBusConnection::sessionBus(),
                                                            QDBusServiceWatcher::WatchForRegistration, this );
    QEventLoop loop;
    connect( watcher, SIGNAL( serviceRegistered( const QString& ) ), &loop, SLOT( quit() ) );
    QProcess::startDetached( "kmail-mobile" );
    loop.exec();

    delete watcher;

    interface = new QDBusInterface( "org.kde.kmailmobile.composer", "/composer" );
  }

  interface->call( "openComposer", emailAddresses.join( ", " ), QString(), QString(), QString(), QString() );

  delete interface;
}

void MainView::finishEdit( QObject *editor )
{
  mOpenItemEditors.remove( editor );
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
  if ( mOpenItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  ContactEditorView *editor = new ContactEditorView;
  editor->loadContact( item );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL( destroyed( QObject* ) ), SLOT( finishEdit( QObject* ) ) );
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );

  editor->show();
}

void MainView::editContactGroup( const Akonadi::Item &item )
{
  if ( mOpenItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  ContactGroupEditorView *editor = new ContactGroupEditorView;
  editor->loadContactGroup( item );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL( destroyed( QObject* ) ), SLOT( finishEdit( QObject* ) ) );
  connect( editor, SIGNAL( requestLaunchAccountWizard() ), SLOT( launchAccountWizard() ) );

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

  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContact ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContact() ) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContactGroup() ) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::EditItem ), SIGNAL( triggered( bool ) ),
           this, SLOT( editItem() ) );
  connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );
  connect( mActionManager, SIGNAL( actionStateUpdated() ), SLOT( updateActionTexts() ) );

  ActionHelper::adaptStandardActionTexts( mActionManager );

  mActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Address Book" ) );
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

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize All Accounts" ) );
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
    actionCollection()->action( "akonadi_item_copy" )->setText( ki18np( "Copy Contact", "Copy %1 Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_copy_to_dialog" )->setText( i18n( "Copy Contact To" ) );
    actionCollection()->action( "akonadi_item_delete" )->setText( ki18np( "Delete Contact", "Delete %1 Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_move_to_dialog" )->setText( i18n( "Move Contact To" ) );
    actionCollection()->action( "akonadi_contact_item_edit" )->setText( i18n( "Edit Contact" ) );
  } else if ( mimeType == KABC::ContactGroup::mimeType() ) {
    actionCollection()->action( "akonadi_item_copy" )->setText( ki18np( "Copy Group Of Contacts", "Copy %1 Groups Of Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_copy_to_dialog" )->setText( i18n( "Copy Group Of Contacts To" ) );
    actionCollection()->action( "akonadi_item_delete" )->setText( ki18np( "Delete Group Of Contacts", "Delete %1 Groups Of Contacts" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_move_to_dialog" )->setText( i18n( "Move Group Of Contacts To" ) );
    actionCollection()->action( "akonadi_contact_item_edit" )->setText( i18n( "Edit Group Of Contacts" ) );
  }
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

#include "mainview.moc"
