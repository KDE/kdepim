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


#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>

#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionview.h>
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

#include "contacteditordialog.h"
#include "contactfiltermodel.h"
#include "contactgroupeditordialog.h"
#include "quicksearchwidget.h"
#include "xxportmanager.h"

MainWidget::MainWidget( KXmlGuiWindow *guiWindow, QWidget *parent )
  : QWidget( parent ),
    mGuiWindow( guiWindow )
{
  mContactModel = new Akonadi::KABCModel( this );

  mCollectionModel = new Akonadi::CollectionModel( this );
  mCollectionModel->setHeaderData( 0, Qt::Horizontal, i18nc( "@title:column, contact groups", "Group" ) , Qt::EditRole );

  mCollectionFilterModel = new Akonadi::CollectionFilterProxyModel();
  mCollectionFilterModel->addMimeTypeFilter( KABC::Addressee::mimeType() );
  mCollectionFilterModel->addMimeTypeFilter( KABC::ContactGroup::mimeType() );
  mCollectionFilterModel->setSourceModel( mCollectionModel );

  // display collections sorted
  QSortFilterProxyModel *sortModel = new QSortFilterProxyModel( this );
  sortModel->setDynamicSortFilter( true );
  sortModel->setSortCaseSensitivity( Qt::CaseInsensitive );
  sortModel->setSourceModel( mCollectionFilterModel );

  mXXPortManager = new XXPortManager( sortModel, this );

  setupGui();
  setupActions();

  mCollectionView->setModel( sortModel );
  mCollectionView->setXmlGuiWindow( guiWindow );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  ContactFilterModel *contactFilterModel = new ContactFilterModel( this );
  contactFilterModel->setSourceModel( mContactModel );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           contactFilterModel, SLOT( setFilterString( const QString& ) ) );

  mItemView->setModel( contactFilterModel );
  mItemView->setXmlGuiWindow( guiWindow );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );
  for ( int column = 1; column < mContactModel->columnCount(); ++column )
    mItemView->setColumnHidden( column, true );

  connect( mCollectionView, SIGNAL( currentChanged( const Akonadi::Collection& ) ),
           this, SLOT( collectionSelected( const Akonadi::Collection& ) ) );
  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
  connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );

  // show the contact details view as default
  mDetailsViewStack->setCurrentWidget( mContactDetails );

  Akonadi::Control::widgetNeedsAkonadi( this );

  mActionManager = new Akonadi::StandardActionManager( guiWindow->actionCollection(), guiWindow );
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
  mCollectionView = new Akonadi::CollectionView();
  splitter->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::ItemView;
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

void MainWidget::setupActions()
{
  KAction *action = 0;
  KActionCollection *collection = mGuiWindow->actionCollection();

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

  action = collection->addAction( "file_import_opera" );
  action->setText( i18n( "Import Opera Address Book..." ) );
  mXXPortManager->addImportAction( action, "opera" );

  action = collection->addAction( "file_import_eudora" );
  action->setText( i18n( "Import Eudora Address Book..." ) );
  mXXPortManager->addImportAction( action, "eudora" );

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

void MainWidget::newContact()
{
  ContactEditorDialog dlg( ContactEditorDialog::CreateMode, mCollectionFilterModel, this );
  dlg.exec();
}

void MainWidget::newGroup()
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::CreateMode, mCollectionFilterModel, this );
  dlg.exec();
}

void MainWidget::editItem( const Akonadi::Item &reference )
{
  const QModelIndex index = mContactModel->indexForItem( reference, 0 );
  const Akonadi::Item item = mContactModel->itemForIndex( index );

  if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::Addressee::mimeType() ) ) {
    editContact( reference );
  } else if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::ContactGroup::mimeType() ) ) {
    editGroup( reference );
  }
}

void MainWidget::collectionSelected( const Akonadi::Collection &collection )
{
  mContactModel->setCollection( collection );
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
  ContactEditorDialog dlg( ContactEditorDialog::EditMode, mCollectionFilterModel, this );
  dlg.setContact( contact );
  dlg.exec();
}

void MainWidget::editGroup( const Akonadi::Item &group )
{
  ContactGroupEditorDialog dlg( ContactGroupEditorDialog::EditMode, mCollectionFilterModel, this );
  dlg.setContactGroup( group );
  dlg.exec();
}

#include "mainwidget.moc"
