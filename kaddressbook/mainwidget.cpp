/*
    This file is part of KAddressBook.

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
#include <akonadi/descendantsproxymodel.h>
#include <akonadi/entityfilterproxymodel.h>
#include <akonadi/itemview.h>
#include <akonadi/mimetypechecker.h>
#include <akonadi/selectionproxymodel.h>
#include <akonadi/entitytreeviewstatesaver.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kabc/contactlineedit.h>
#include <kabc/kabcmodel.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kxmlguiwindow.h>

#include "akonadi/contact/contacteditordialog.h"
#include "akonadi/contact/contactgroupeditordialog.h"
#include "akonadi/contact/contactgroupviewer.h"
#include "akonadi/contact/contactviewer.h"
#include "akonadi_next/entitytreeview.h"
#include "contactfiltermodel.h"
#include "contactstreemodel.h"
#include "contactswitcher.h"
#include "globalcontactmodel.h"
#include "modelcolumnmanager.h"
#include "quicksearchwidget.h"
#include "settings.h"
#include "standardcontactactionmanager.h"
#include "xxportmanager.h"

#include "printing/printingwizard.h"

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *parent )
  : QWidget( parent )
{
  mXXPortManager = new XXPortManager( this );

  setupGui();
  setupActions( guiClient->actionCollection() );

  /*
   *  The item models, proxies and views have the following structure:
   *
   *                               mItemView
   *                                   ^
   *                                   |
   *                           contactFilterModel
   *                                   ^
   *                                   |
   *                               mItemTree
   *                                   ^
   *                                   |
   *                             mDescendantTree
   *                                   ^
   *                                   |
   *      mCollectionView  ->  selectionProxyModel
   *            ^                      ^
   *            |                      |
   *      mCollectionTree              |
   *            ^                      |
   *            |                      |
   *             \                    /
   *            GlobalContactModel::instance()
   *
   *
   *  GlobalContactModel::instance():  The global contact model (contains collections and items)
   *                 mCollectionTree:  Filters out all items
   *                 mCollectionView:  Shows the collections (address books) in a view
   *             selectionProxyModel:  Filters out all collections and items that are no children
   *                                   of the collection currently selected in mCollectionView
   *                 mDescendantTree:  Flattens the item/collection tree to a list
   *                       mItemTree:  Filters out all collections
   *              contactFilterModel:  Filters the contacts by the content of mQuickSearchWidget
   *                       mItemView:  Shows the items (contacts and contact groups) in a view
   *
   *                       descModel:  Flattens the item/collection tree to a list
   *         mContactCompletionModel:  Provides a list of all contacts that can be used for auto-completion
   */

  mCollectionTree = new Akonadi::EntityFilterProxyModel( this );
  mCollectionTree->setSourceModel( GlobalContactModel::instance()->model() );
  mCollectionTree->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionTree->setHeaderSet( Akonadi::EntityTreeModel::CollectionTreeHeaders );

  mXXPortManager->setCollectionModel( mCollectionTree );

  mCollectionView->setModel( mCollectionTree );
  mCollectionView->setXmlGuiClient( guiClient );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  Akonadi::SelectionProxyModel *selectionProxyModel = new Akonadi::SelectionProxyModel( mCollectionView->selectionModel(),
                                                                                        this );
  selectionProxyModel->setSourceModel( GlobalContactModel::instance()->model() );

  mDescendantTree = new Akonadi::DescendantsProxyModel( this );
  mDescendantTree->setSourceModel( selectionProxyModel );

  mItemTree = new Akonadi::EntityFilterProxyModel( this );
  mItemTree->setSourceModel( mDescendantTree );
  mItemTree->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  mItemTree->setHeaderSet( Akonadi::EntityTreeModel::ItemListHeaders );

  ContactFilterModel *contactFilterModel = new ContactFilterModel( this );
  contactFilterModel->setSourceModel( mItemTree );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           contactFilterModel, SLOT( setFilterString( const QString& ) ) );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           this, SLOT( selectFirstItem() ) );
  connect( mQuickSearchWidget, SIGNAL( arrowDownKeyPressed() ),
           mItemView, SLOT( setFocus() ) );

  mItemView->setModel( contactFilterModel );
  mItemView->setXmlGuiClient( guiClient );
  mItemView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mItemView->setRootIsDecorated( false );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );

  mXXPortManager->setSelectionModel( mItemView->selectionModel() );

  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
  connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );

  // show the contact details view as default
  mDetailsViewStack->setCurrentWidget( mContactDetails );

  mContactSwitcher->setView( mItemView );

  Akonadi::Control::widgetNeedsAkonadi( this );

  mActionManager = new Akonadi::StandardContactActionManager( guiClient->actionCollection(), this );
  mActionManager->setCollectionSelectionModel( mCollectionView->selectionModel() );
  mActionManager->setItemSelectionModel( mItemView->selectionModel() );

  mActionManager->createAllActions();

  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContact ), SIGNAL( triggered( bool ) ),
           this, SLOT( newContact() ) );
  connect( mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup ), SIGNAL( triggered( bool ) ),
           this, SLOT( newGroup() ) );
  connect( mActionManager, SIGNAL( editItem( const Akonadi::Item& ) ),
           this, SLOT( editItem( const Akonadi::Item& ) ) );

  mModelColumnManager = new ModelColumnManager( GlobalContactModel::instance()->model(), this );
  mModelColumnManager->setWidget( mItemView->header() );
  mModelColumnManager->load();

  // restore previous state
  const KConfigGroup cfg( Settings::self()->config(), "CollectionViewState" );
  Akonadi::EntityTreeViewStateSaver *restorer = new Akonadi::EntityTreeViewStateSaver( mCollectionView );
  restorer->restoreState( cfg );
}

MainWidget::~MainWidget()
{
  mModelColumnManager->store();
  KConfigGroup cfg( Settings::self()->config(), "CollectionViewState" );
  Akonadi::EntityTreeViewStateSaver saver( mCollectionView );
  saver.saveState( cfg );
  cfg.sync();
  Settings::self()->writeConfig();
}

void MainWidget::setupGui()
{
  // the horizontal main layout
  QHBoxLayout *layout = new QHBoxLayout( this );

  // The splitter that contains the three main parts of the gui
  //   - collection view on the left
  //   - item view in the middle
  //   - details pane on the right, that contains
  //       - details view stack on the top
  //       - contact switcher at the bottom
  QSplitter *splitter = new QSplitter;
  layout->addWidget( splitter );

  // the collection view
  mCollectionView = new Akonadi::EntityTreeView();
  splitter->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::EntityTreeView();
  splitter->addWidget( mItemView );

  // the details pane that contains the details view stack and contact switcher
  mDetailsPane = new QWidget;
  splitter->addWidget( mDetailsPane );

  QVBoxLayout *detailsPaneLayout = new QVBoxLayout( mDetailsPane );
  detailsPaneLayout->setMargin( 0 );

  // the details view stack
  mDetailsViewStack = new QStackedWidget();
  detailsPaneLayout->addWidget( mDetailsViewStack );

  // the details widget for contacts
  mContactDetails = new Akonadi::ContactViewer( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactDetails );

  // the details widget for contact groups
  mContactGroupDetails = new Akonadi::ContactGroupViewer( mDetailsViewStack );
  mDetailsViewStack->addWidget( mContactGroupDetails );

  // the contact switcher for the simple gui mode
  mContactSwitcher = new ContactSwitcher;
  detailsPaneLayout->addWidget( mContactSwitcher );
  mContactSwitcher->setVisible( false );

  // the quick search widget which is embedded in the toolbar action
  mQuickSearchWidget = new QuickSearchWidget;
}

void MainWidget::setupActions( KActionCollection *collection )
{
  KAction *action = 0;
  KToggleAction *toggleAction = 0;

  action = KStandardAction::print( this, SLOT( print() ), collection );
  action->setWhatsThis( i18n( "Print a special number of contacts." ) );

  action = collection->addAction( "quick_search" );
  action->setText( i18n( "Quick search" ) );
  action->setDefaultWidget( mQuickSearchWidget );

  action = collection->addAction( "select_all" );
  action->setText( i18n( "Select All" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_A ) );
  connect( action, SIGNAL( triggered( bool ) ), mItemView, SLOT( selectAll() ) );

  toggleAction = collection->add<KToggleAction>( "options_show_collectionview" );
  toggleAction->setText( i18n( "Show Address Books View" ) );
  toggleAction->setWhatsThis( i18n( "Toggle whether the address books view shall be visible." ) );
  toggleAction->setCheckedState( KGuiItem( i18n( "Hide Address Books View" ) ) );
  toggleAction->setChecked( true );
  connect( toggleAction, SIGNAL( toggled( bool ) ), SLOT( setCollectionViewVisible( bool ) ) );

  toggleAction = collection->add<KToggleAction>( "options_show_itemview" );
  toggleAction->setText( i18n( "Show Contacts View" ) );
  toggleAction->setWhatsThis( i18n( "Toggle whether the contacts view shall be visible." ) );
  toggleAction->setCheckedState( KGuiItem( i18n( "Hide Contacts View" ) ) );
  toggleAction->setChecked( true );
  connect( toggleAction, SIGNAL( toggled( bool ) ), SLOT( setItemViewVisible( bool ) ) );

  toggleAction = collection->add<KToggleAction>( "options_show_detailsview" );
  toggleAction->setText( i18n( "Show Details View" ) );
  toggleAction->setWhatsThis( i18n( "Toggle whether the details view shall be visible." ) );
  toggleAction->setCheckedState( KGuiItem( i18n( "Hide Details View" ) ) );
  toggleAction->setChecked( true );
  connect( toggleAction, SIGNAL( toggled( bool ) ), SLOT( setDetailsViewVisible( bool ) ) );

  toggleAction = collection->add<KToggleAction>( "options_show_simplegui" );
  toggleAction->setText( i18n( "Show Simple View" ) );
  connect( toggleAction, SIGNAL( toggled( bool ) ), SLOT( setSimpleGuiMode( bool ) ) );

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

  action = collection->addAction( "file_import_ldap" );
  action->setText( i18n( "Import from LDAP server..." ) );
  mXXPortManager->addImportAction( action, "ldap" );

  action = collection->addAction( "file_import_gmx" );
  action->setText( i18n( "Import GMX file..." ) );
  mXXPortManager->addImportAction( action, "gmx" );


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

  action = collection->addAction( "file_export_gmx" );
  action->setText( i18n( "Export GMX file..." ) );
  mXXPortManager->addExportAction( action, "gmx" );

}

void MainWidget::print()
{
  QPrinter printer;
  printer.setDocName( i18n( "Address Book" ) );
  printer.setOutputFileName( "addressbook.pdf" );
  printer.setOutputFormat( QPrinter::PdfFormat );

  QPrintDialog printDialog( &printer, this );
  printDialog.setWindowTitle( i18n( "Print Contacts" ) );
  if ( !printDialog.exec() )
    return;

  KABPrinting::PrintingWizard wizard( &printer, mItemView, this );
  wizard.exec();
}

void MainWidget::newContact()
{
  Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::CreateMode, this );
  dlg.exec();
}

void MainWidget::newGroup()
{
  Akonadi::ContactGroupEditorDialog dlg( Akonadi::ContactGroupEditorDialog::CreateMode, this );
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

/**
 * Depending on the mime type of the selected item, this method
 * brings up the right view on the detail view stack and sets the
 * selected item on it.
 */
void MainWidget::itemSelected( const Akonadi::Item &item )
{
  if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::Addressee::mimeType() ) ) {
    mDetailsViewStack->setCurrentWidget( mContactDetails );
    mContactDetails->setContact( item );
  } else if ( Akonadi::MimeTypeChecker::isWantedItem( item, KABC::ContactGroup::mimeType() ) ) {
    mDetailsViewStack->setCurrentWidget( mContactGroupDetails );
    mContactGroupDetails->setContactGroup( item );
  }
}

void MainWidget::selectFirstItem()
{
  // Whenever the quick search has changed, we select the first item
  // in the item view, so that the detailsview is updated
  if ( mItemView && mItemView->selectionModel() ) {
    mItemView->selectionModel()->setCurrentIndex( mItemView->model()->index( 0, 0 ),
                                                  QItemSelectionModel::Rows |
                                                  QItemSelectionModel::ClearAndSelect );
  }
}

void MainWidget::setCollectionViewVisible( bool visible )
{
  mCollectionView->setVisible( visible );
}

void MainWidget::setItemViewVisible( bool visible )
{
  mItemView->setVisible( visible );
}

void MainWidget::setDetailsViewVisible( bool visible )
{
  mDetailsPane->setVisible( visible );
}

void MainWidget::setSimpleGuiMode( bool on )
{
  mCollectionView->setVisible( !on );
  mItemView->setVisible( !on );
  mDetailsPane->setVisible( true );
  mContactSwitcher->setVisible( on );

  if ( mCollectionView->model() )
    mCollectionView->setCurrentIndex( mCollectionView->model()->index( 0, 0 ) );

  if ( mItemView->model() )
    mItemView->setCurrentIndex( mItemView->model()->index( 0, 0 ) );
}

void MainWidget::editContact( const Akonadi::Item &contact )
{
  Akonadi::ContactEditorDialog dlg( Akonadi::ContactEditorDialog::EditMode, this );
  dlg.setContact( contact );
  dlg.exec();
}

void MainWidget::editGroup( const Akonadi::Item &group )
{
  Akonadi::ContactGroupEditorDialog dlg( Akonadi::ContactGroupEditorDialog::EditMode, this );
  dlg.setContactGroup( group );
  dlg.exec();
}

#include "mainwidget.moc"
