/*
    This file is part of KContactManager.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

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

#include "mainwidget.h"
#include <QPrinter>
#include <QPrintDialog>

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/control.h>
#include <akonadi/itemview.h>
#include <akonadi/mimetypechecker.h>
#include <akonadi/standardactionmanager.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kabc/contactgroupbrowser.h>
#include <kabc/contactlineedit.h>
#include <kabc/kabcmodel.h>
#include <kabc/kabcitembrowser.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktoolbar.h>
#include <kxmlguiwindow.h>

#include "akonadi_next/descendantentitiesproxymodel.h"
#include "akonadi_next/entityfilterproxymodel.h"
#include "akonadi_next/entitytreeview.h"
#include "contacteditordialog.h"
#include "contactfiltermodel.h"
#include "contactgroupeditordialog.h"
#include "contactstreemodel.h"
#include "globalcontactmodel.h"
#include "kcontactmanageradaptor.h"
#include "quicksearchwidget.h"
#include "xxportmanager.h"

#include "printing/printingwizard.h"

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *parent )
  : QWidget( parent )
{
  mXXPortManager = new XXPortManager( this );

  setupGui();
  setupActions( guiClient->actionCollection() );

  mCollectionTree = new Akonadi::EntityFilterProxyModel( this );
  mCollectionTree->setSourceModel( GlobalContactModel::instance()->model() );
  mCollectionTree->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionTree->setHeaderSet( Akonadi::EntityTreeModel::CollectionTreeHeaders );

  mXXPortManager->setCollectionModel( mCollectionTree );

  mCollectionView->setModel( mCollectionTree );
  mCollectionView->setXmlGuiClient( guiClient );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

/*
  ContactFilterModel *contactFilterModel = new ContactFilterModel( this );
  contactFilterModel->setSourceModel( mContactModel );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           contactFilterModel, SLOT( setFilterString( const QString& ) ) );
*/

  mDescendantTree = new Akonadi::DescendantEntitiesProxyModel( this );
  mDescendantTree->setSourceModel( GlobalContactModel::instance()->model() );

  mItemTree = new Akonadi::EntityFilterProxyModel( this );
  mItemTree->setSourceModel( mDescendantTree );
  mItemTree->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  mItemTree->setHeaderSet( Akonadi::EntityTreeModel::ItemListHeaders );

  mItemView->setModel( mItemTree );
  //mItemView->setXmlGuiClient( guiClient );
  mItemView->setRootIsDecorated( false );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );
  for ( int column = 1; column < mDescendantTree->columnCount( QModelIndex() ); ++column )
    mItemView->setColumnHidden( column, true );

  connect( mCollectionView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
           this, SLOT( collectionSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );

  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
  connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );

  // show the contact details view as default
  mDetailsViewStack->setCurrentWidget( mContactDetails );

  Akonadi::Control::widgetNeedsAkonadi( this );

  mActionManager = new Akonadi::StandardActionManager( guiClient->actionCollection(), this );
  mActionManager->setCollectionSelectionModel( mCollectionView->selectionModel() );
  mActionManager->setItemSelectionModel( mItemView->selectionModel() );

  mActionManager->createAllActions();
  mActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add Address Book" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::CopyCollections, ki18np( "Copy Address Book", "Copy %1 Address Books" ) );
  mActionManager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete Address Book" ) );
  mActionManager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Reload" ) );
  mActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Properties..." ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::CopyItems, ki18np( "Copy Contact", "Copy %1 Contacts" ) );
  mActionManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete Contact", "Delete %1 Contacts" ) );
  new MainWidgetAdaptor( this );
  QDBusConnection::sessionBus().registerObject( "/KContactManager", this, QDBusConnection::ExportAdaptors );

}

MainWidget::~MainWidget()
{
}

void MainWidget::setupGui()
{
  // the horizontal main layout
  QHBoxLayout *layout = new QHBoxLayout( this );

  // the splitter that contains the three main parts of the gui
  //   - collection view on the left
  //   - item view in the middle
  //   - details view on the right
  QSplitter *splitter = new QSplitter;
  layout->addWidget( splitter );

  // the collection view
  mCollectionView = new Akonadi::EntityTreeView();
  splitter->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::EntityTreeView();
  splitter->addWidget( mItemView );

  // the details view stack
  mDetailsViewStack = new QStackedWidget();
  splitter->addWidget( mDetailsViewStack );

  // the details widget for contacts
  mContactDetails = new Akonadi::KABCItemBrowser( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactDetails );

  // the details widget for contact groups
  mContactGroupDetails = new Akonadi::ContactGroupBrowser( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactGroupDetails );

  mQuickSearchWidget = new QuickSearchWidget;
}

void MainWidget::setupActions( KActionCollection *collection )
{
  KAction *action = 0;

  action = collection->addAction( "file_new_contact" );
  action->setIcon( KIcon( "contact-new" ) );
  action->setText( i18n( "&New Contact..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newContact() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
  action->setWhatsThis( i18n( "Create a new contact<p>You will be presented with a dialog where you can add all data about a person, including addresses and phone numbers.</p>" ) );

  action = collection->addAction( "file_new_group" );
  action->setIcon( KIcon( "user-group-new" ) );
  action->setText( i18n( "&New Group..." ) );
  connect( action, SIGNAL( triggered(bool) ), SLOT( newGroup() ));
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_G ) );
  action->setWhatsThis( i18n( "Create a new group<p>You will be presented with a dialog where you can add a new group of contacts.</p>" ) );


  action = KStandardAction::print( this, SLOT( print() ), collection );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  action = collection->addAction( "quick_search" );
  action->setText( i18n( "Quick search" ) );
  action->setDefaultWidget( mQuickSearchWidget );

  // import actions
  action = collection->addAction( "file_import_vcard" );
  action->setText( i18n( "Import vCard..." ) );
  mXXPortManager->addImportAction( action, "vcard30" );

  action = collection->addAction( "file_import_csv" );
  action->setText( i18n( "Import CSV file..." ) );
  mXXPortManager->addImportAction( action, "csv" );

  action = collection->addAction( "file_import_ldif" );
  action->setText( i18n( "Import LDIF file..." ) );
  mXXPortManager->addImportAction( action, "ldif" );

  // export actions
  action = collection->addAction( "file_export_vcard30" );
  action->setText( i18n( "Export vCard 3.0..." ) );
  mXXPortManager->addExportAction( action, "vcard30" );

  action = collection->addAction( "file_export_vcard21" );
  action->setText( i18n( "Export vCard 2.1..." ) );
  mXXPortManager->addExportAction( action, "vcard21" );

  action = collection->addAction( "file_export_csv" );
  action->setText( i18n( "Export CSV file..." ) );
  mXXPortManager->addExportAction( action, "csv" );

  action = collection->addAction( "file_export_ldif" );
  action->setText( i18n( "Export LDIF file..." ) );
  mXXPortManager->addExportAction( action, "ldif" );
}

void MainWidget::print()
{
  QPrinter printer;
  printer.setDocName( i18n( "Address Book" ) );
  printer.setOutputFileName( "addressbook.pdf" );
  printer.setOutputFormat( QPrinter::PdfFormat );

  QPrintDialog printDialog( &printer, this );
  printDialog.setWindowTitle( i18n( "Print Addresses" ) );
  if ( !printDialog.exec() )
    return;
#if 0
  KABPrinting::PrintingWizard wizard( &printer, mAddressBook,
                                      mViewManager->selectedUids(), this );

  wizard.exec();
#endif
}

void MainWidget::newContact()
{
  ContactEditorDialog dlg( ContactEditorDialog::CreateMode, mCollectionTree, this );
  dlg.exec();
}

void MainWidget::newGroup()
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::CreateMode, mCollectionTree, this );
  dlg.exec();
}

void MainWidget::editItem( const Akonadi::Item &reference )
{
  if ( Akonadi::MimeTypeChecker::isWantedItem( reference, KABC::Addressee::mimeType() ) ) {
    editContact( reference );
  } else if ( Akonadi::MimeTypeChecker::isWantedItem( reference, KABC::ContactGroup::mimeType() ) ) {
    editGroup( reference );
  }
}

void MainWidget::collectionSelectionChanged( const QItemSelection &selected, const QItemSelection& )
{
  const QModelIndex index = selected.indexes().at( 0 );

  const QModelIndex sourceIndex = mCollectionTree->mapToSource( index );

  mDescendantTree->setRootIndex( sourceIndex );

  const QModelIndex itemIndex = mDescendantTree->mapFromSource( sourceIndex );

  mItemTree->setRootIndex( itemIndex );

  const QModelIndex viewIndex = mItemTree->mapFromSource( itemIndex );

  mItemView->setRootIndex( viewIndex );

  mActionManager->setItemSelectionModel( mItemView->selectionModel() );
}

/**
 * Depending on the mime type of the selected item, this method
 * brings up the right view on the detail view stack and sets the
 * selected item on it.
 */
void MainWidget::itemSelected( const Akonadi::Item &item )
{
  if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::Addressee::mimeType() ) ) {
    mDetailsViewStack->setCurrentWidget( mContactDetails );
    mContactDetails->setItem( item );
  } else if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::ContactGroup::mimeType() ) ) {
    mDetailsViewStack->setCurrentWidget( mContactGroupDetails );
    mContactGroupDetails->setItem( item );
  }
}

void MainWidget::editContact( const Akonadi::Item &contact )
{
  ContactEditorDialog dlg( ContactEditorDialog::EditMode, mCollectionTree, this );
  dlg.setContact( contact );
  dlg.exec();
}

void MainWidget::editGroup( const Akonadi::Item &group )
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::EditMode, mCollectionTree, this );
  dlg.setContactGroup( group );
  dlg.exec();
}

#include "mainwidget.moc"
