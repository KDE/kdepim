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

#include "contactswitcher.h"
#include "globalcontactmodel.h"
#include <kdescendantsproxymodel.h>
#include "modelcolumnmanager.h"
#include "printing/printingwizard.h"
#include "quicksearchwidget.h"
#include "settings.h"
#include "xxportmanager.h"

#ifdef GRANTLEE_FOUND
#include "grantleecontactformatter.h"
#include "grantleecontactgroupformatter.h"
#endif

#include <akonadi_next/kcheckableproxymodel.h>
#include <akonadi/etmviewstatesaver.h>
#include <akonadi/collectionfilterproxymodel.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/contact/contactdefaultactions.h>
#include <akonadi/contact/contacteditordialog.h>
#include <akonadi/contact/contactgroupeditordialog.h>
#include <akonadi/contact/contactgroupviewer.h>
#include <akonadi/contact/contactsfilterproxymodel.h>
#include <akonadi/contact/contactstreemodel.h>
#include <akonadi/contact/contactviewer.h>
#include <akonadi/contact/standardcontactactionmanager.h>
#include <akonadi/control.h>
#include <akonadi/entitymimetypefiltermodel.h>
#include <akonadi/entitytreeview.h>
#include <akonadi/entitytreeviewstatesaver.h>
#include <akonadi/itemview.h>
#include <akonadi/mimetypechecker.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kapplication.h>
#include <kicon.h>
#include <klineedit.h>
#include <klocale.h>
#include <kselectionproxymodel.h>
#include <kstandarddirs.h>
#include <ktextbrowser.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <kxmlguiwindow.h>
#include <libkdepim/uistatesaver.h>

#include <QtGui/QAction>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QListView>
#include <QtGui/QPrinter>
#include <QtGui/QPrintDialog>
#include <QtGui/QSortFilterProxyModel>
#include <QtGui/QSplitter>
#include <QtGui/QStackedWidget>



namespace {
static bool isStructuralCollection( const Akonadi::Collection &collection )
{
  QStringList mimeTypes;
  mimeTypes << KABC::Addressee::mimeType() << KABC::ContactGroup::mimeType();
  const QStringList collectionMimeTypes = collection.contentMimeTypes();
  foreach ( const QString &mimeType, mimeTypes ) {
    if ( collectionMimeTypes.contains( mimeType ) )
      return false;
  }
  return true;
}

class StructuralCollectionsNotCheckableProxy : public Future::KCheckableProxyModel {
public:
  StructuralCollectionsNotCheckableProxy(QObject* parent)
      :Future::KCheckableProxyModel(parent)
  { }

  /* reimp */ QVariant data( const QModelIndex &index, int role ) const
  {
    if ( !index.isValid() )
      return QVariant();

    if ( role == Qt::CheckStateRole ) {
      // Don't show the checkbox if the collection can't contain incidences
      const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
      if ( collection.isValid() && isStructuralCollection( collection ) ) {
        return QVariant();
      }
    }
    return Future::KCheckableProxyModel::data( index, role );
  }
};

}

MainWidget::MainWidget( KXMLGUIClient *guiClient, QWidget *parent )
  : QWidget( parent ), mAllContactsModel( 0 ), mXmlGuiClient( guiClient )
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
   *                           mContactsFilterModel
   *                                   ^
   *                                   |
   *                               mItemTree
   *                                   ^
   *                                   |
   *                                   |           mAllContactsModel
   *                                   |                  ^
   *                                   |                  |
   *      mCollectionView     selectionProxyModel  descendantsModel
   *            ^               ^      ^                  ^
   *            |               |      |                  |
   *            |       selectionModel |                  |
   *            |               |      |                  |
   *        proxyModel ---------'      |                  |
   *            ^                      |                  |
   *            |                      |                  |
   *      mCollectionTree              |                  |
   *            ^                      |                  |
   *            |                      |   _______________/
   *             \                    /  /
   *            GlobalContactModel::instance()
   *
   *
   *  GlobalContactModel::instance():  The global contact model (contains collections and items)
   *                 mCollectionTree:  Filters out all items
   *                      proxyModel:  Allows the user to select collections by checkboxes
   *                  selectionModel:  Represents the selected collections that have been selected in
   *                                   proxyModel
   *                 mCollectionView:  Shows the collections (address books) in a view
   *             selectionProxyModel:  Filters out all collections and items that are no children
   *                                   of the collections currently selected in selectionModel
   *                       mItemTree:  Filters out all collections
   *            mContactsFilterModel:  Filters the contacts by the content of mQuickSearchWidget
   *                       mItemView:  Shows the items (contacts and contact groups) in a view
   *
   *                descendantsModel:  Flattens the item/collection tree to a list
   *               mAllContactsModel:  Provides a list of all available contacts from all address books
   */

  mCollectionTree = new Akonadi::EntityMimeTypeFilterModel( this );
  mCollectionTree->setDynamicSortFilter( true );
  mCollectionTree->setSortCaseSensitivity( Qt::CaseInsensitive );
  mCollectionTree->setSourceModel( GlobalContactModel::instance()->model() );
  mCollectionTree->addMimeTypeInclusionFilter( Akonadi::Collection::mimeType() );
  mCollectionTree->setHeaderGroup( Akonadi::EntityTreeModel::CollectionTreeHeaders );

  mCollectionSelectionModel = new QItemSelectionModel( mCollectionTree );
  StructuralCollectionsNotCheckableProxy *checkableProxyModel = new StructuralCollectionsNotCheckableProxy( this );
  checkableProxyModel->setSelectionModel( mCollectionSelectionModel );
  checkableProxyModel->setSourceModel( mCollectionTree );

  mCollectionView->setModel( checkableProxyModel );
  mCollectionView->setXmlGuiClient( guiClient );
  mCollectionView->header()->setDefaultAlignment( Qt::AlignCenter );
  mCollectionView->header()->setSortIndicatorShown( false );

  connect( mCollectionView, SIGNAL( currentChanged( const Akonadi::Collection& ) ),
           mXXPortManager, SLOT( setDefaultAddressBook( const Akonadi::Collection& ) ) );

  KSelectionProxyModel *selectionProxyModel = new KSelectionProxyModel( mCollectionSelectionModel,
                                                                        this );
  selectionProxyModel->setSourceModel( GlobalContactModel::instance()->model() );
  selectionProxyModel->setFilterBehavior( KSelectionProxyModel::ChildrenOfExactSelection );

  mItemTree = new Akonadi::EntityMimeTypeFilterModel( this );
  mItemTree->setSourceModel( selectionProxyModel );
  mItemTree->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
  mItemTree->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );

  mContactsFilterModel = new Akonadi::ContactsFilterProxyModel( this );
  mContactsFilterModel->setSourceModel( mItemTree );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           mContactsFilterModel, SLOT( setFilterString( const QString& ) ) );
  connect( mQuickSearchWidget, SIGNAL( filterStringChanged( const QString& ) ),
           this, SLOT( selectFirstItem() ) );
  connect( mQuickSearchWidget, SIGNAL( arrowDownKeyPressed() ),
           mItemView, SLOT( setFocus() ) );

  mItemView->setModel( mContactsFilterModel );
  mItemView->setXmlGuiClient( guiClient );
  mItemView->setSelectionMode( QAbstractItemView::ExtendedSelection );
  mItemView->setRootIsDecorated( false );
  mItemView->header()->setDefaultAlignment( Qt::AlignCenter );

  mXXPortManager->setSelectionModel( mItemView->selectionModel() );

  mActionManager = new Akonadi::StandardContactActionManager( guiClient->actionCollection(), this );
  mActionManager->setCollectionSelectionModel( mCollectionView->selectionModel() );
  mActionManager->setItemSelectionModel( mItemView->selectionModel() );
  mActionManager->createAllActions();

  connect( mItemView, SIGNAL( currentChanged( const Akonadi::Item& ) ),
           this, SLOT( itemSelected( const Akonadi::Item& ) ) );
  connect( mItemView, SIGNAL( doubleClicked( const Akonadi::Item& ) ),
           mActionManager->action( Akonadi::StandardContactActionManager::EditItem ), SLOT( trigger() ) );
  connect( mItemView->selectionModel(), SIGNAL( currentChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( itemSelectionChanged( const QModelIndex&, const QModelIndex& ) ) );

  // show the contact details view as default
  mDetailsViewStack->setCurrentWidget( mContactDetails );

  mContactSwitcher->setView( mItemView );

  Akonadi::Control::widgetNeedsAkonadi( this );

  mModelColumnManager = new ModelColumnManager( GlobalContactModel::instance()->model(), this );
  mModelColumnManager->setWidget( mItemView->header() );
  mModelColumnManager->load();

  QMetaObject::invokeMethod( this, "delayedInit", Qt::QueuedConnection );
}

void MainWidget::delayedInit()
{
  // restore previous state
  {
    const KConfigGroup group( Settings::self()->config(), "UiState_MainWidgetSplitter" );
    KPIM::UiStateSaver::restoreState( mMainWidgetSplitter, group );
  }
  {
    const KConfigGroup group( Settings::self()->config(), "UiState_ContactView" );
    KPIM::UiStateSaver::restoreState( mItemView, group );
  }

  mXmlGuiClient->actionCollection()->action( "options_show_simplegui" )->setChecked( Settings::self()->useSimpleMode() );

  connect( GlobalContactModel::instance()->model(), SIGNAL( modelAboutToBeReset() ), SLOT( saveState() ) );
  connect( GlobalContactModel::instance()->model(), SIGNAL( modelReset() ), SLOT( restoreState() ) );
  connect( kapp, SIGNAL( aboutToQuit() ), SLOT( saveState() ) );

  restoreState();
}

MainWidget::~MainWidget()
{
  mModelColumnManager->store();

  {
    if ( !Settings::self()->useSimpleMode() ) {
      // Do not save the splitter values when in simple mode, because we can't
      // restore them correctly when switching back to normal mode

      KConfigGroup group( Settings::self()->config(), "UiState_MainWidgetSplitter" );
      KPIM::UiStateSaver::saveState( mMainWidgetSplitter, group );
    }
  }
  {
    KConfigGroup group( Settings::self()->config(), "UiState_ContactView" );
    KPIM::UiStateSaver::saveState( mItemView, group );
  }

  saveState();

  Settings::self()->writeConfig();
}

void MainWidget::restoreState()
{
  // collection view
  {
    Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
    saver->setView( mCollectionView );

    const KConfigGroup group( Settings::self()->config(), "CollectionViewState" );
    saver->restoreState( group );
  }

  // collection view
  {
    Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
    saver->setSelectionModel( mCollectionSelectionModel );

    const KConfigGroup group( Settings::self()->config(), "CollectionViewCheckState" );
    saver->restoreState( group );
  }

  // item view
  {
    Akonadi::ETMViewStateSaver *saver = new Akonadi::ETMViewStateSaver;
    saver->setView( mItemView );
    saver->setSelectionModel( mItemView->selectionModel() );

    const KConfigGroup group( Settings::self()->config(), "ItemViewState" );
    saver->restoreState( group );
  }
}

void MainWidget::saveState()
{
  // collection view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setView( mCollectionView );

    KConfigGroup group( Settings::self()->config(), "CollectionViewState" );
    saver.saveState( group );
    group.sync();
  }

  // collection view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setSelectionModel( mCollectionSelectionModel );

    KConfigGroup group( Settings::self()->config(), "CollectionViewCheckState" );
    saver.saveState( group );
    group.sync();
  }

  // item view
  {
    Akonadi::ETMViewStateSaver saver;
    saver.setView( mItemView );
    saver.setSelectionModel( mItemView->selectionModel() );

    KConfigGroup group( Settings::self()->config(), "ItemViewState" );
    saver.saveState( group );
    group.sync();
  }
}

void MainWidget::setupGui()
{
  // the horizontal main layout
  QHBoxLayout *layout = new QHBoxLayout( this );
  layout->setMargin( 0 );

  // The splitter that contains the three main parts of the gui
  //   - collection view on the left
  //   - item view in the middle
  //   - details pane on the right, that contains
  //       - details view stack on the top
  //       - contact switcher at the bottom
  mMainWidgetSplitter = new QSplitter;
  mMainWidgetSplitter->setObjectName( "MainWidgetSplitter" );

  layout->addWidget( mMainWidgetSplitter );

  // the collection view
  mCollectionView = new Akonadi::EntityTreeView();
  mMainWidgetSplitter->addWidget( mCollectionView );

  // the items view
  mItemView = new Akonadi::EntityTreeView();
  mItemView->setObjectName( "ContactView" );
  mMainWidgetSplitter->addWidget( mItemView );

  // the details pane that contains the details view stack and contact switcher
  mDetailsPane = new QWidget;
  mMainWidgetSplitter->addWidget( mDetailsPane );

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

  // the details widget for empty items
  mEmptyDetails = new KTextBrowser( mDetailsViewStack );
  mDetailsViewStack->addWidget( mEmptyDetails );

  // the contact switcher for the simple gui mode
  mContactSwitcher = new ContactSwitcher;
  detailsPaneLayout->addWidget( mContactSwitcher );
  mContactSwitcher->setVisible( false );

  // the quick search widget which is embedded in the toolbar action
  mQuickSearchWidget = new QuickSearchWidget;

  // setup the default actions
  Akonadi::ContactDefaultActions *actions = new Akonadi::ContactDefaultActions( this );
  actions->connectToView( mContactDetails );
  actions->connectToView( mContactGroupDetails );

#if 0 // disabled because Grantlee supports no i18n for KDE 4.6 yet
 Akonadi::GrantleeContactFormatter *formatter =
       new Akonadi::GrantleeContactFormatter( KStandardDirs::locate( "data", QLatin1String( "kaddressbook/viewertemplates/" ) ) );

 mContactDetails->setContactFormatter( formatter );

 Akonadi::GrantleeContactGroupFormatter *groupFormatter =
       new Akonadi::GrantleeContactGroupFormatter( KStandardDirs::locate( "data", QLatin1String( "kaddressbook/viewertemplates/" ) ) );

 mContactGroupDetails->setContactGroupFormatter( groupFormatter );
#endif
}

void MainWidget::setupActions( KActionCollection *collection )
{
  KAction *action = 0;
  KToggleAction *toggleAction = 0;

  action = KStandardAction::print( this, SLOT( print() ), collection );
  action->setWhatsThis( i18n( "Print the complete address book or a selected number of contacts." ) );

  action = collection->addAction( "quick_search" );
  action->setText( i18n( "Quick search" ) );
  action->setDefaultWidget( mQuickSearchWidget );

  action = collection->addAction( "select_all" );
  action->setText( i18n( "Select All" ) );
  action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_A ) );
  action->setWhatsThis( i18n( "Select all contacts in the current address book view." ) );
  connect( action, SIGNAL( triggered( bool ) ), mItemView, SLOT( selectAll() ) );

  toggleAction = collection->add<KToggleAction>( "options_show_simplegui" );
  toggleAction->setText( i18n( "Show Simple View" ) );
  action->setWhatsThis( i18n( "Show a simple mode of the address book view." ) );
  connect( toggleAction, SIGNAL( toggled( bool ) ), SLOT( setSimpleGuiMode( bool ) ) );

  // import actions
  action = collection->addAction( "file_import_vcard" );
  action->setText( i18n( "Import vCard..." ) );
  action->setWhatsThis( i18n( "Import contacts from a vCard file." ) );
  mXXPortManager->addImportAction( action, "vcard30" );

  action = collection->addAction( "file_import_csv" );
  action->setText( i18n( "Import CSV file..." ) );
  action->setWhatsThis( i18n( "Import contacts from a file in comma separated value format." ) );
  mXXPortManager->addImportAction( action, "csv" );

  action = collection->addAction( "file_import_ldif" );
  action->setText( i18n( "Import LDIF file..." ) );
  action->setWhatsThis( i18n( "Import contacts from an LDIF file." ) );
  mXXPortManager->addImportAction( action, "ldif" );

  action = collection->addAction( "file_import_ldap" );
  action->setText( i18n( "Import from LDAP server..." ) );
  action->setWhatsThis( i18n( "Import contacts from an LDAP server." ) );
  mXXPortManager->addImportAction( action, "ldap" );

  action = collection->addAction( "file_import_gmx" );
  action->setText( i18n( "Import GMX file..." ) );
  action->setWhatsThis( i18n( "Import contacts from a GMX address book file." ) );
  mXXPortManager->addImportAction( action, "gmx" );


  // export actions
  action = collection->addAction( "file_export_vcard30" );
  action->setText( i18n( "Export vCard 3.0..." ) );
  action->setWhatsThis( i18n( "Export contacts to a vCard 3.0 file." ) );
  mXXPortManager->addExportAction( action, "vcard30" );

  action = collection->addAction( "file_export_vcard21" );
  action->setText( i18n( "Export vCard 2.1..." ) );
  action->setWhatsThis( i18n( "Export contacts to a vCard 2.1 file." ) );
  mXXPortManager->addExportAction( action, "vcard21" );

  action = collection->addAction( "file_export_csv" );
  action->setText( i18n( "Export CSV file..." ) );
  action->setWhatsThis( i18n( "Export contacts to a file in comma separated value format." ) );
  mXXPortManager->addExportAction( action, "csv" );

  action = collection->addAction( "file_export_ldif" );
  action->setText( i18n( "Export LDIF file..." ) );
  action->setWhatsThis( i18n( "Export contacts to an LDIF file." ) );
  mXXPortManager->addExportAction( action, "ldif" );

  action = collection->addAction( "file_export_gmx" );
  action->setText( i18n( "Export GMX file..." ) );
  action->setWhatsThis( i18n( "Export contacts to a GMX address book file." ) );
  mXXPortManager->addExportAction( action, "gmx" );
}

void MainWidget::print()
{
  QPrinter printer;
  printer.setDocName( i18n( "Address Book" ) );
  printer.setOutputFileName( "addressbook.pdf" );
  printer.setOutputFormat( QPrinter::PdfFormat );
  printer.setCollateCopies( true );

  QPrintDialog printDialog( &printer, this );
  printDialog.setWindowTitle( i18n( "Print Contacts" ) );
  if ( !printDialog.exec() ) //krazy:exclude=crashy
    return;

  KABPrinting::PrintingWizard wizard( &printer, mItemView->selectionModel(), this );
  wizard.setDefaultAddressBook( currentAddressBook() );

  wizard.exec();
}

void MainWidget::newContact()
{
  mActionManager->action( Akonadi::StandardContactActionManager::CreateContact )->trigger();
}

void MainWidget::newGroup()
{
  mActionManager->action( Akonadi::StandardContactActionManager::CreateContactGroup )->trigger();
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

/**
 * Catch when the selection has gone ( e.g. an empty address book has been selected )
 * clear the details view in this case.
 */
void MainWidget::itemSelectionChanged( const QModelIndex &current, const QModelIndex& )
{
  if ( !current.isValid() )
    mDetailsViewStack->setCurrentWidget( mEmptyDetails );
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

void MainWidget::setSimpleGuiMode( bool on )
{
  mCollectionView->setVisible( !on );
  mItemView->setVisible( !on );
  mDetailsPane->setVisible( true );
  mContactSwitcher->setVisible( on );

  if ( mItemView->model() )
    mItemView->setCurrentIndex( mItemView->model()->index( 0, 0 ) );

  Settings::self()->setUseSimpleMode( on );
}

Akonadi::Collection MainWidget::currentAddressBook() const
{
  if ( mCollectionView->selectionModel() && mCollectionView->selectionModel()->hasSelection() ) {
    const QModelIndex index = mCollectionView->selectionModel()->selectedIndexes().first();
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole )
                                                .value<Akonadi::Collection>();

    return collection;
  }

  return Akonadi::Collection();
}

QAbstractItemModel* MainWidget::allContactsModel()
{
  if ( !mAllContactsModel ) {
    KDescendantsProxyModel *descendantsModel = new KDescendantsProxyModel( this );
    descendantsModel->setSourceModel( GlobalContactModel::instance()->model() );

    mAllContactsModel = new Akonadi::EntityMimeTypeFilterModel( this );
    mAllContactsModel->setSourceModel( descendantsModel );
    mAllContactsModel->addMimeTypeExclusionFilter( Akonadi::Collection::mimeType() );
    mAllContactsModel->setHeaderGroup( Akonadi::EntityTreeModel::ItemListHeaders );
  }

  return mAllContactsModel;
}

#include "mainwidget.moc"
