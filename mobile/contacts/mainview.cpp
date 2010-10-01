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
#include "contactsexporthandler.h"
#include "contactsimporthandler.h"

#include <akonadi/agentactionmanager.h>
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
#include <QtDeclarative/QDeclarativeEngine>

QML_DECLARE_TYPE( Akonadi::Contact::ContactViewItem )
QML_DECLARE_TYPE( Akonadi::Contact::ContactGroupViewItem )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "kaddressbook-mobile", new ContactListProxy, parent )
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
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  actionCollection()->addAction( QLatin1String( "import_vcards" ), action );

  action = new KAction( i18n( "Export Contacts" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
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

ImportHandlerBase* MainView::importHandler() const
{
  return new ContactsImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new ContactsExportHandler();
}

#include "mainview.moc"
