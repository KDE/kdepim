/*
  This file is part of KOrganizer.

  Copyright (c) 2000,2001,2003 Cornelius Schumacher <schumacher@kde.org>
  Copyright (c) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  Copyright (c) 2005 Rafal Rzepecki <divide@users.sourceforge.net>
  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2013 Sérgio Martins <iamsergio@gmail.com>

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

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "todoview.h"

#include "incidencetreemodel.h"
#include "tododelegates.h"
#include "todomodel.h"
#include "todoviewsortfilterproxymodel.h"
#include "todoviewquickaddline.h"
#include "todoviewquicksearch.h"
#include "todoviewview.h"
#include "helper.h"

#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <libkdepim/widgets/kdatepickerpopup.h>

#include <entitymimetypefiltermodel.h>
#include <ETMViewStateSaver>
#include <TagFetchJob>
#include <Tag>

#include <KCalCore/CalFormat>
#include <KIcon>
#include <KIconLoader>
#include <KGlobal>
#include <KComponentData>
#include <KJob>
#include <QDebug>

#include <QCheckBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QMenu>
#include <QToolButton>
#include <QItemSelection>
#include <QTimer>

Q_DECLARE_METATYPE(QPointer<QMenu>)

using namespace EventViews;
using namespace KCalCore;

namespace EventViews {
// We share this struct between all views, for performance and memory purposes
class ModelStack
{
public:
  ModelStack( const EventViews::PrefsPtr &preferences, QObject *parent_ )
    : todoModel( new TodoModel( preferences ) ),
      parent( parent_ ),
      calendar( 0 ),
      todoTreeModel( 0 ),
      todoFlatModel( 0 ),
      prefs( preferences )
  {
  }

  ~ModelStack()
  {
    delete todoModel;
    delete todoTreeModel;
    delete todoFlatModel;
  }

  void registerView( TodoView *view )
  {
    views << view;
  }

  void unregisterView( TodoView *view )
  {
    views.removeAll( view );
  }

  void setFlatView( bool flat )
  {
    const QString todoMimeType = QLatin1String( "application/x-vnd.akonadi.calendar.todo" );
    if ( flat ) {
      foreach ( TodoView *view, views ) {
        // In flatview dropping confuses users and it's very easy to drop into a child item
        view->mView->setDragDropMode( QAbstractItemView::DragOnly );
        view->setFlatView( flat, /**propagate=*/false ); // So other views update their toggle icon

        if ( todoTreeModel ) {
          view->saveViewState(); // Save the tree state before it's gone
        }
      }

      delete todoFlatModel;
      todoFlatModel = new Akonadi::EntityMimeTypeFilterModel( parent );
      todoFlatModel->addMimeTypeInclusionFilter( todoMimeType );
      todoFlatModel->setSourceModel( calendar ? calendar->model() : 0 );
      todoModel->setSourceModel( todoFlatModel );

      delete todoTreeModel;
      todoTreeModel = 0;
    } else {
      delete todoTreeModel;
      todoTreeModel = new IncidenceTreeModel( QStringList() << todoMimeType, parent );
      foreach ( TodoView *view, views ) {
        QObject::connect( todoTreeModel, SIGNAL(indexChangedParent(QModelIndex)),
                          view, SLOT(expandIndex(QModelIndex)) );
        QObject::connect( todoTreeModel, SIGNAL(batchInsertionFinished()),
                          view, SLOT(restoreViewState()) );
        view->mView->setDragDropMode( QAbstractItemView::DragDrop );
        view->setFlatView( flat, /**propagate=*/false ); // So other views update their toggle icon
      }
      todoTreeModel->setSourceModel( calendar ? calendar->model() : 0 );
      todoModel->setSourceModel( todoTreeModel );
      delete todoFlatModel;
      todoFlatModel = 0;
    }

    foreach ( TodoView *view, views ) {
      view->mFlatViewButton->blockSignals( true );
      // We block signals to avoid recursion, we have two TodoViews and mFlatViewButton is synchronized
      view->mFlatViewButton->setChecked( flat );
      view->mFlatViewButton->blockSignals( false );
      view->mView->setRootIsDecorated( !flat );
      view->restoreViewState();
    }

    prefs->setFlatListTodo( flat );
    prefs->writeConfig();
  }

  void setCalendar( const Akonadi::ETMCalendar::Ptr &newCalendar )
  {
    calendar = newCalendar;
    todoModel->setCalendar( calendar );
    if ( todoTreeModel ) {
      todoTreeModel->setSourceModel( calendar ? calendar->model() : 0 );
    }
  }

  bool isFlatView() const
  {
    return todoFlatModel != 0;
  }

  TodoModel *todoModel;
  QList<TodoView*> views;
  QObject *parent;

  Akonadi::ETMCalendar::Ptr calendar;
  IncidenceTreeModel *todoTreeModel;
  Akonadi::EntityMimeTypeFilterModel *todoFlatModel;
  EventViews::PrefsPtr prefs;
};
}

// Don't use K_GLOBAL_STATIC, see QTBUG-22667
static ModelStack *sModels = 0;

TodoView::TodoView( const EventViews::PrefsPtr &prefs,
                    bool sidebarView, QWidget *parent )
  : EventView( parent )
  , mQuickSearch( 0 )
  , mQuickAdd( 0 )
  , mTreeStateRestorer( 0 )
  , mSidebarView( sidebarView )
  , mResizeColumnsScheduled( false )
{
  mResizeColumnsTimer = new QTimer( this );
  connect( mResizeColumnsTimer, SIGNAL(timeout()), SLOT(resizeColumns()) );
  mResizeColumnsTimer->setInterval( 100 ); // so we don't overdue it when user resizes window manually
  mResizeColumnsTimer->setSingleShot( true );

  setPreferences( prefs );
  if ( !sModels ) {
    sModels = new ModelStack( prefs, parent );
  }
  sModels->registerView( this );

  mProxyModel = new TodoViewSortFilterProxyModel( preferences(), this );
  mProxyModel->setSourceModel( sModels->todoModel );
  mProxyModel->setDynamicSortFilter( true );
  mProxyModel->setFilterKeyColumn( TodoModel::SummaryColumn );
  mProxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel->setSortRole( Qt::EditRole );
  connect( mProxyModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onRowsInserted(QModelIndex,int,int)) );

  if ( !mSidebarView ) {
    mQuickSearch = new TodoViewQuickSearch( calendar(), this );
    mQuickSearch->setVisible( prefs->enableTodoQuickSearch() );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             mProxyModel, SLOT(setFilterRegExp(QString)) );
    connect( mQuickSearch, SIGNAL(searchTextChanged(QString)),
             SLOT(restoreViewState()) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             mProxyModel, SLOT(setCategoryFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterCategoryChanged(QStringList)),
             SLOT(restoreViewState()) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             mProxyModel, SLOT(setPriorityFilter(QStringList)) );
    connect( mQuickSearch, SIGNAL(filterPriorityChanged(QStringList)),
             SLOT(restoreViewState()) );
  }

  mView = new TodoViewView( this );
  mView->setModel( mProxyModel );

  mView->setContextMenuPolicy( Qt::CustomContextMenu );

  mView->setSortingEnabled( true );

  mView->setAutoExpandDelay( 250 );
  mView->setDragDropMode( QAbstractItemView::DragDrop );

  mView->setExpandsOnDoubleClick( false );
  mView->setEditTriggers( QAbstractItemView::SelectedClicked |
                          QAbstractItemView::EditKeyPressed );

  connect( mView->header(), SIGNAL(geometriesChanged()), SLOT(scheduleResizeColumns()) );
  connect( mView, SIGNAL(visibleColumnCountChanged()), SLOT(resizeColumns()) );

  TodoRichTextDelegate *richTextDelegate = new TodoRichTextDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::SummaryColumn, richTextDelegate );
  mView->setItemDelegateForColumn( TodoModel::DescriptionColumn, richTextDelegate );

  TodoPriorityDelegate *priorityDelegate = new TodoPriorityDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::PriorityColumn, priorityDelegate );

  TodoDueDateDelegate *startDateDelegate = new TodoDueDateDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::StartDateColumn, startDateDelegate );

  TodoDueDateDelegate *dueDateDelegate = new TodoDueDateDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::DueDateColumn, dueDateDelegate );

  TodoCompleteDelegate *completeDelegate = new TodoCompleteDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::PercentColumn, completeDelegate );

  mCategoriesDelegate = new TodoCategoriesDelegate( mView );
  mView->setItemDelegateForColumn( TodoModel::CategoriesColumn, mCategoriesDelegate );

  connect( mView, SIGNAL(customContextMenuRequested(QPoint)),
           this, SLOT(contextMenu(QPoint)) );
  connect( mView, SIGNAL(doubleClicked(QModelIndex)),
           this, SLOT(itemDoubleClicked(QModelIndex)) );

  connect( mView->selectionModel(),
           SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
           this,
           SLOT(selectionChanged(QItemSelection,QItemSelection)) );

  mQuickAdd = new TodoViewQuickAddLine( this );
  mQuickAdd->setClearButtonShown( true );
  mQuickAdd->setVisible( preferences()->enableQuickTodo() );
  connect( mQuickAdd, SIGNAL(returnPressed(Qt::KeyboardModifiers)),
           this, SLOT(addQuickTodo(Qt::KeyboardModifiers)) );

  mFullViewButton = 0;
  if ( !mSidebarView ) {
    mFullViewButton = new QToolButton( this );
    mFullViewButton->setAutoRaise( true );
    mFullViewButton->setCheckable( true );

    mFullViewButton->setToolTip(
      i18nc( "@info:tooltip",
             "Display to-do list in a full window" ) );
    mFullViewButton->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Checking this option will cause the to-do view to use the full window." ) );
  }
  mFlatViewButton = new QToolButton( this );
  mFlatViewButton->setAutoRaise( true );
  mFlatViewButton->setCheckable( true );
  mFlatViewButton->setToolTip(
    i18nc( "@info:tooltip",
           "Display to-dos in flat list instead of a tree" ) );
  mFlatViewButton->setWhatsThis(
    i18nc( "@info:whatsthis",
           "Checking this option will cause the to-dos to be displayed as a "
           "flat list instead of a hierarchical tree; the parental "
           "relationships are removed in the display." ) );

  connect( mFlatViewButton, SIGNAL(toggled(bool)), SLOT(setFlatView(bool)) );
  if ( mFullViewButton ) {
    connect( mFullViewButton, SIGNAL(toggled(bool)), SLOT(setFullView(bool)) );
  }

  QGridLayout *layout = new QGridLayout( this );
  layout->setMargin( 0 );
  if ( !mSidebarView ) {
    layout->addWidget( mQuickSearch, 0, 0, 1, 2 );
  }
  layout->addWidget( mView, 1, 0, 1, 2 );
  layout->setRowStretch( 1, 1 );
  layout->addWidget( mQuickAdd, 2, 0 );

  // Dummy layout just to add a few px of right margin so the checkbox is aligned
  // with the QAbstractItemView's viewport.
  QHBoxLayout *dummyLayout = new QHBoxLayout();
  dummyLayout->setContentsMargins( 0, 0, mView->frameWidth()/*right*/, 0 );
  if ( !mSidebarView ) {
    QFrame *f = new QFrame( this );
    f->setFrameShape( QFrame::VLine );
    f->setFrameShadow( QFrame::Sunken );
    dummyLayout->addWidget( f );
    dummyLayout->addWidget( mFullViewButton );
  }
  dummyLayout->addWidget( mFlatViewButton );

  layout->addLayout( dummyLayout, 2, 1 );
  setLayout( layout );

  // ---------------- POPUP-MENUS -----------------------
  mItemPopupMenu = new QMenu( this );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    i18nc( "@action:inmenu show the to-do", "&Show" ),
    this, SLOT(showTodo()) );

  QAction *a = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu edit the to-do", "&Edit..." ),
    this, SLOT(editTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();
  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    cachedSmallIcon( QLatin1String("document-print") ),
    i18nc( "@action:inmenu print the to-do", "&Print..." ),
    this, SIGNAL(printTodo()) );

  mItemPopupMenuItemOnlyEntries << mItemPopupMenu->addAction(
    cachedSmallIcon( QLatin1String("document-print-preview") ),
    i18nc( "@action:inmenu print preview the to-do", "Print Previe&w..." ),
    this, SIGNAL(printPreviewTodo()) );

  mItemPopupMenu->addSeparator();
  a = mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon( QLatin1String("edit-delete"), KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18nc( "@action:inmenu delete the to-do", "&Delete" ),
    this, SLOT(deleteTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mItemPopupMenu->addSeparator();

  mItemPopupMenu->addAction(
    KIconLoader::global()->loadIcon(
      QLatin1String("view-calendar-tasks"), KIconLoader::NoGroup, KIconLoader::SizeSmall ),
    i18nc( "@action:inmenu create a new to-do", "New &To-do..." ),
    this, SLOT(newTodo()) );

  a = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu create a new sub-to-do", "New Su&b-to-do..." ),
    this, SLOT(newSubTodo()) );
  mItemPopupMenuReadWriteEntries << a;
  mItemPopupMenuItemOnlyEntries << a;

  mMakeTodoIndependent = mItemPopupMenu->addAction(
    i18nc( "@action:inmenu", "&Make this To-do Independent" ),
    this, SIGNAL(unSubTodoSignal()) );

  mMakeSubtodosIndependent =
    mItemPopupMenu->addAction(
      i18nc( "@action:inmenu", "Make all Sub-to-dos &Independent" ),
      this, SIGNAL(unAllSubTodoSignal()) );

  mItemPopupMenuItemOnlyEntries << mMakeTodoIndependent;
  mItemPopupMenuItemOnlyEntries << mMakeSubtodosIndependent;

  mItemPopupMenuReadWriteEntries << mMakeTodoIndependent;
  mItemPopupMenuReadWriteEntries << mMakeSubtodosIndependent;

  mItemPopupMenu->addSeparator();

  mCopyPopupMenu =
    new KPIM::KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                KPIM::KDatePickerPopup::DatePicker |
                                KPIM::KDatePickerPopup::Words,
                                QDate::currentDate(), this );
  mCopyPopupMenu->setTitle( i18nc( "@title:menu", "&Copy To" ) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(copyTodoToDate(QDate)) );

  connect( mCopyPopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mMovePopupMenu =
    new KPIM:: KDatePickerPopup( KPIM::KDatePickerPopup::NoDate |
                                 KPIM::KDatePickerPopup::DatePicker |
                                 KPIM::KDatePickerPopup::Words,
                                 QDate::currentDate(), this );
  mMovePopupMenu->setTitle( i18nc( "@title:menu", "&Move To" ) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           SLOT(setNewDate(QDate)) );

  connect( mMovePopupMenu, SIGNAL(dateChanged(QDate)),
           mItemPopupMenu, SLOT(hide()) );

  mItemPopupMenu->insertMenu( 0, mCopyPopupMenu );
  mItemPopupMenu->insertMenu( 0, mMovePopupMenu );

  mItemPopupMenu->addSeparator();
  mItemPopupMenu->addAction(
    i18nc( "@action:inmenu delete completed to-dos", "Pur&ge Completed" ),
    this, SIGNAL(purgeCompletedSignal()) );

  mPriorityPopupMenu = new QMenu( this );
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu unspecified priority", "unspecified" ) ) ] = 0;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu highest priority", "1 (highest)" ) ) ] = 1;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=2", "2" ) ) ] = 2;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=3", "3" ) ) ] = 3;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=4", "4" ) ) ] = 4;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu medium priority", "5 (medium)" ) ) ] = 5;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=6", "6" ) ) ] = 6;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=7", "7" ) ) ] = 7;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu priority value=8", "8" ) ) ] = 8;
  mPriority[ mPriorityPopupMenu->addAction(
               i18nc( "@action:inmenu lowest priority", "9 (lowest)" ) ) ] = 9;
  connect( mPriorityPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPriority(QAction*)) );

  mPercentageCompletedPopupMenu = new QMenu(this);
  for ( int i = 0; i <= 100; i+=10 ) {
    const QString label = QString::fromLatin1( "%1 %" ).arg( i );
    mPercentage[mPercentageCompletedPopupMenu->addAction( label )] = i;
  }
  connect( mPercentageCompletedPopupMenu, SIGNAL(triggered(QAction*)),
           SLOT(setNewPercentage(QAction*)) );

  setMinimumHeight( 50 );

  // Initialize our proxy models
  setFlatView( preferences()->flatListTodo() );
  setFullView( preferences()->fullViewTodo() );

  updateConfig();
}

TodoView::~TodoView()
{
  saveViewState();

  sModels->unregisterView( this );
  if ( sModels->views.isEmpty() ) {
    delete sModels;
    sModels = 0;
  }
}

void TodoView::expandIndex( const QModelIndex &index )
{
  QModelIndex todoModelIndex = sModels->todoModel->mapFromSource( index );
  Q_ASSERT( todoModelIndex.isValid() );
  QModelIndex realIndex = mProxyModel->mapFromSource( todoModelIndex );
  Q_ASSERT( realIndex.isValid() );
  while ( realIndex.isValid() ) {
    mView->expand( realIndex );
    realIndex = mProxyModel->parent( realIndex );
  }
}

void TodoView::setCalendar( const Akonadi::ETMCalendar::Ptr &calendar )
{
  EventView::setCalendar( calendar );

  if ( !mSidebarView ) {
    mQuickSearch->setCalendar( calendar );
  }
  mCategoriesDelegate->setCalendar( calendar );
  sModels->setCalendar( calendar );
  restoreViewState();
}

Akonadi::Item::List TodoView::selectedIncidences() const
{
  Akonadi::Item::List ret;
  const QModelIndexList selection = mView->selectionModel()->selectedRows();
  Q_FOREACH ( const QModelIndex &mi, selection ) {
    ret << mi.data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  }
  return ret;
}

DateList TodoView::selectedIncidenceDates() const
{
  // The todo view only lists todo's. It's probably not a good idea to
  // return something about the selected todo here, because it has got
  // a couple of dates (creation, due date, completion date), and the
  // caller could not figure out what he gets. So just return an empty list.
  return DateList();
}

void TodoView::saveLayout( KConfig *config, const QString &group ) const
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility;
  QVariantList columnOrder;
  QVariantList columnWidths;
  for ( int i = 0; i < header->count(); ++i ) {
    columnVisibility << QVariant( !mView->isColumnHidden( i ) );
    columnWidths << QVariant( header->sectionSize( i ) );
    columnOrder << QVariant( header->visualIndex( i ) );
  }
  cfgGroup.writeEntry( "ColumnVisibility", columnVisibility );
  cfgGroup.writeEntry( "ColumnOrder", columnOrder );
  cfgGroup.writeEntry( "ColumnWidths", columnWidths );

  cfgGroup.writeEntry( "SortAscending", (int)header->sortIndicatorOrder() );
  if ( header->isSortIndicatorShown() ) {
    cfgGroup.writeEntry( "SortColumn", header->sortIndicatorSection() );
  } else {
    cfgGroup.writeEntry( "SortColumn", -1 );
  }

  if ( !mSidebarView ) {
    preferences()->setFullViewTodo( mFullViewButton->isChecked() );
  }
  preferences()->setFlatListTodo( mFlatViewButton->isChecked() );
}

void TodoView::restoreLayout( KConfig *config, const QString &group, bool minimalDefaults )
{
  KConfigGroup cfgGroup = config->group( group );
  QHeaderView *header = mView->header();

  QVariantList columnVisibility = cfgGroup.readEntry( "ColumnVisibility", QVariantList() );
  QVariantList columnOrder = cfgGroup.readEntry( "ColumnOrder", QVariantList() );
  QVariantList columnWidths = cfgGroup.readEntry( "ColumnWidths", QVariantList() );

  if ( columnVisibility.isEmpty() ) {
    // if config is empty then use default settings
    mView->hideColumn( TodoModel::RecurColumn );
    mView->hideColumn( TodoModel::DescriptionColumn );
    mView->hideColumn( TodoModel::CalendarColumn );

    if ( minimalDefaults ) {
      mView->hideColumn( TodoModel::PriorityColumn );
      mView->hideColumn( TodoModel::PercentColumn );
      mView->hideColumn( TodoModel::DescriptionColumn );
      mView->hideColumn( TodoModel::CategoriesColumn );
    }

    // We don't have any incidences (content) yet, so we delay resizing
    QTimer::singleShot( 0, this, SLOT(resizeColumns()) );

  } else {
      for ( int i = 0;
            i < header->count() &&
            i < columnOrder.size() &&
            i < columnWidths.size() &&
            i < columnVisibility.size();
            i++ ) {
      bool visible = columnVisibility[i].toBool();
      int width = columnWidths[i].toInt();
      int order = columnOrder[i].toInt();

      header->resizeSection( i, width );
      header->moveSection( header->visualIndex( i ), order );
      if ( i != 0 && !visible ) {
        mView->hideColumn( i );
      }
    }
  }

  int sortOrder = cfgGroup.readEntry( "SortAscending", (int)Qt::AscendingOrder );
  int sortColumn = cfgGroup.readEntry( "SortColumn", -1 );
  if ( sortColumn >= 0 ) {
    mView->sortByColumn( sortColumn, (Qt::SortOrder)sortOrder );
  }

  mFlatViewButton->setChecked( cfgGroup.readEntry( "FlatView", false ) );
}

void TodoView::setIncidenceChanger( Akonadi::IncidenceChanger *changer )
{
  EventView::setIncidenceChanger( changer );
  sModels->todoModel->setIncidenceChanger( changer );
}

void TodoView::showDates( const QDate &start, const QDate &end, const QDate & )
{
  // There is nothing to do here for the Todo View
  Q_UNUSED( start );
  Q_UNUSED( end );
}

void TodoView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void TodoView::updateView()
{
  // View is always updated, it's connected to ETM.
}

void TodoView::changeIncidenceDisplay( const Akonadi::Item &, Akonadi::IncidenceChanger::ChangeType )
{
  // Don't do anything, model is connected to ETM, it's up to date
}

void TodoView::updateConfig()
{
  Q_ASSERT( preferences() );
  if ( !mSidebarView && mQuickSearch ) {
    mQuickSearch->setVisible( preferences()->enableTodoQuickSearch() );
  }

  if ( mQuickAdd )
    mQuickAdd->setVisible( preferences()->enableQuickTodo() );

  updateView();
}

void TodoView::clearSelection()
{
  mView->selectionModel()->clearSelection();
}

void TodoView::addTodo( const QString &summary,
                        const Akonadi::Item &parentItem,
                        const QStringList &categories )
{
  if ( !changer() || summary.trimmed().isEmpty() ) {
    return;
  }

  KCalCore::Todo::Ptr parent = CalendarSupport::todo( parentItem );

  KCalCore::Todo::Ptr todo( new KCalCore::Todo );
  todo->setSummary( summary.trimmed() );
  todo->setOrganizer(
    Person::Ptr( new Person( CalendarSupport::KCalPrefs::instance()->fullName(),
                             CalendarSupport::KCalPrefs::instance()->email() ) ) );

  todo->setCategories( categories );

  if ( parent && !parent->hasRecurrenceId() ) {
    todo->setRelatedTo( parent->uid() );
  }

  Akonadi::Collection collection;

  // Use the same collection of the parent.
  if ( parentItem.isValid() ) {
      // Don't use parentColection() since it might be a virtual collection
      collection = calendar()->collection( parentItem.storageCollectionId() );
  }

  changer()->createIncidence( todo, Akonadi::Collection(), this );
}

void TodoView::addQuickTodo( Qt::KeyboardModifiers modifiers )
{
  if ( modifiers == Qt::NoModifier ) {
    /*const QModelIndex index = */ addTodo( mQuickAdd->text(), Akonadi::Item(),
                                            mProxyModel->categories() );

  } else if ( modifiers == Qt::ControlModifier ) {
    QModelIndexList selection = mView->selectionModel()->selectedRows();
    if ( selection.count() != 1 ) {
      qWarning() << "No to-do selected" << selection;
      return;
    }
    const QModelIndex idx = mProxyModel->mapToSource( selection[0] );
    mView->expand( selection[0] );
    const Akonadi::Item parent = sModels->todoModel->data( idx,
                      Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
    addTodo( mQuickAdd->text(), parent, mProxyModel->categories() );
  } else {
    return;
  }
  mQuickAdd->setText( QString() );
}

void TodoView::contextMenu( const QPoint &pos )
{
  const bool hasItem = mView->indexAt( pos ).isValid();
  Incidence::Ptr incidencePtr;

  Q_FOREACH ( QAction *entry, mItemPopupMenuItemOnlyEntries ) {
    bool enable;

    if ( hasItem ) {
      const Akonadi::Item::List incidences = selectedIncidences();

      if ( incidences.isEmpty() ) {
        enable = false;
      } else {
        Akonadi::Item item = incidences.first();
        incidencePtr = CalendarSupport::incidence( item );

        // Action isn't RO, it can change the incidence, "Edit" for example.
        const bool actionIsRw = mItemPopupMenuReadWriteEntries.contains( entry );

        const bool incidenceIsRO = !calendar()->hasRight( item, Akonadi::Collection::CanChangeItem );

        enable = hasItem && ( !actionIsRw ||
                              ( actionIsRw && !incidenceIsRO ) );

      }
    } else {
      enable = false;
    }

    entry->setEnabled( enable );
  }
  mCopyPopupMenu->setEnabled( hasItem );
  mMovePopupMenu->setEnabled( hasItem );

  if ( hasItem ) {
    if ( incidencePtr ) {
      const bool hasRecId = incidencePtr->hasRecurrenceId();
      if ( calendar() ) {
        mMakeSubtodosIndependent->setEnabled( !hasRecId && !calendar()->childItems( incidencePtr->uid() ).isEmpty() );
      }
      mMakeTodoIndependent->setEnabled( !hasRecId && !incidencePtr->relatedTo().isEmpty() );
    }

    switch ( mView->indexAt( pos ).column() ) {
    case TodoModel::PriorityColumn:
      mPriorityPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case TodoModel::PercentColumn:
      mPercentageCompletedPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case TodoModel::StartDateColumn:
    case TodoModel::DueDateColumn:
      mMovePopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    case TodoModel::CategoriesColumn:
      createCategoryPopupMenu()->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    default:
      mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
      break;
    }
  } else {
    mItemPopupMenu->popup( mView->viewport()->mapToGlobal( pos ) );
  }
}

void TodoView::selectionChanged( const QItemSelection &selected,
                                   const QItemSelection &deselected )
{
  Q_UNUSED( deselected );
  QModelIndexList selection = selected.indexes();
  if ( selection.isEmpty() || !selection[0].isValid() ) {
    emit incidenceSelected( Akonadi::Item(), QDate() );
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();

  if ( selectedIncidenceDates().isEmpty() ) {
    emit incidenceSelected( todoItem, QDate() );
  } else {
    emit incidenceSelected( todoItem, selectedIncidenceDates().first() );
  }
}

void TodoView::showTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();

  emit showIncidenceSignal( todoItem );
}

void TodoView::editTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  emit editIncidenceSignal( todoItem );
}

void TodoView::deleteTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();

    if ( !changer()->deletedRecently( todoItem.id() ) ) {
      emit deleteIncidenceSignal( todoItem );
    }
  }
}

void TodoView::newTodo()
{
  emit newTodoSignal( QDate::currentDate().addDays( 7 ) );
}

void TodoView::newSubTodo()
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() == 1 ) {
    const Akonadi::Item todoItem =
      selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();

    emit newSubTodoSignal( todoItem );
  } else {
    // This never happens
    qWarning() << "Selection size isn't 1";
  }
}

void TodoView::copyTodoToDate( const QDate &date )
{
  if ( !changer() ) {
    return;
  }

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const QModelIndex origIndex = mProxyModel->mapToSource( selection[0] );
  Q_ASSERT( origIndex.isValid() );

  const Akonadi::Item origItem =
    sModels->todoModel->data( origIndex,
                              Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();

  const KCalCore::Todo::Ptr orig = CalendarSupport::todo( origItem );
  if ( !orig ) {
    return;
  }

  KCalCore::Todo::Ptr todo( orig->clone() );

  todo->setUid( KCalCore::CalFormat::createUniqueId() );

  KDateTime due = todo->dtDue();
  due.setDate( date );
  todo->setDtDue( due );

  changer()->createIncidence( todo, Akonadi::Collection(), this );
}

void TodoView::scheduleResizeColumns()
{
  mResizeColumnsScheduled = true;
  mResizeColumnsTimer->start(); // restarts the timer if already active
}

void TodoView::itemDoubleClicked( const QModelIndex &index )
{
  if ( index.isValid() ) {
    QModelIndex summary = index.sibling( index.row(), TodoModel::SummaryColumn );
    if ( summary.flags() & Qt::ItemIsEditable ) {
      editTodo();
    } else {
      showTodo();
    }
  }
}

QMenu *TodoView::createCategoryPopupMenu()
{
  QMenu *tempMenu = new QMenu( this );

  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return tempMenu;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  const QStringList checkedCategories = todo->categories();


  Akonadi::TagFetchJob *tagFetchJob = new Akonadi::TagFetchJob(this);
  connect(tagFetchJob, SIGNAL(result(KJob*)), this, SLOT(onTagsFetched(KJob*)));
  tagFetchJob->setProperty("menu", QVariant::fromValue(QPointer<QMenu>(tempMenu)));
  tagFetchJob->setProperty("checkedCategories", checkedCategories);

  connect( tempMenu, SIGNAL(triggered(QAction*)),
           SLOT(changedCategories(QAction*)) );
  connect( tempMenu, SIGNAL(aboutToHide()),
           tempMenu, SLOT(deleteLater()) );
  return tempMenu;
}

void TodoView::onTagsFetched(KJob *job)
{
  if (job->error()) {
    qWarning() << "Failed to fetch tags " << job->errorString();
    return;
  }
  Akonadi::TagFetchJob *fetchJob = static_cast<Akonadi::TagFetchJob*>(job);
  const QStringList checkedCategories = job->property("checkedCategories").toStringList();
  QPointer<QMenu> menu = job->property("menu").value<QPointer<QMenu> >();
  if (menu) {
    Q_FOREACH (const Akonadi::Tag &tag, fetchJob->tags()) {
      const QString name = tag.name();
      QAction *action = menu->addAction( name );
      action->setCheckable( true );
      action->setData(name);
      if ( checkedCategories.contains( name ) ) {
        action->setChecked( true );
      }
    }
  }
}


void TodoView::setNewDate( const QDate &date )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    KDateTime dt( date );

    if ( !todo->allDay() ) {
      dt.setTime( todo->dtDue().time() );
    }

    todo->setDtDue( dt );

    changer()->modifyIncidence( todoItem, oldTodo, this );
  } else {
    qDebug() << "Item is readOnly";
  }
}

void TodoView::setNewPercentage( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );

  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    int percentage = mPercentage.value( action );
    if ( percentage == 100 ) {
      todo->setCompleted( KDateTime::currentLocalDateTime() );
      todo->setPercentComplete( 100 );
    } else {
      todo->setPercentComplete( percentage );
    }
    if ( todo->recurs() && percentage == 100 ) {
      changer()->modifyIncidence( todoItem, oldTodo, this );
    } else {
      changer()->modifyIncidence( todoItem, oldTodo, this );
    }
  } else {
    qDebug() << "Item is read only";
  }
}

void TodoView::setNewPriority( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }
  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );
    todo->setPriority( mPriority[action] );

    changer()->modifyIncidence( todoItem, oldTodo, this );
  }
}

void TodoView::changedCategories( QAction *action )
{
  QModelIndexList selection = mView->selectionModel()->selectedRows();
  if ( selection.size() != 1 ) {
    return;
  }

  const Akonadi::Item todoItem = selection[0].data ( TodoModel::TodoRole ).value<Akonadi::Item>();
  KCalCore::Todo::Ptr todo = CalendarSupport::todo( todoItem );
  Q_ASSERT( todo );
  if ( calendar()->hasRight( todoItem, Akonadi::Collection::CanChangeItem ) ) {
    KCalCore::Todo::Ptr oldTodo( todo->clone() );

    const QString cat = action->data().toString();
    QStringList categories = todo->categories();
    if ( categories.contains( cat ) ) {
      categories.removeAll( cat );
    } else {
      categories.append( cat );
    }
    categories.sort();
    todo->setCategories( categories );
    changer()->modifyIncidence( todoItem, oldTodo, this );
  } else {
    qDebug() << "No active item, active item is read-only, or locking failed";
  }
}

void TodoView::setFullView( bool fullView )
{
  if ( !mFullViewButton ) {
    return;
  }

  mFullViewButton->setChecked( fullView );
  if ( fullView ) {
    mFullViewButton->setIcon( KIcon( QLatin1String("view-restore") ) );
  } else {
    mFullViewButton->setIcon( KIcon( QLatin1String("view-fullscreen") ) );
  }

  mFullViewButton->blockSignals( true );
  // We block signals to avoid recursion; there are two TodoViews and
  // also mFullViewButton is synchronized.
  mFullViewButton->setChecked( fullView );
  mFullViewButton->blockSignals( false );

  preferences()->setFullViewTodo( fullView );
  preferences()->writeConfig();

  emit fullViewChanged( fullView );
}

void TodoView::setFlatView( bool flatView, bool notifyOtherViews )
{
  if ( flatView ) {
    mFlatViewButton->setIcon( KIcon( QLatin1String("view-list-tree") ) );
  } else {
    mFlatViewButton->setIcon( KIcon( QLatin1String("view-list-details") ) );
  }

  if ( notifyOtherViews ) {
    sModels->setFlatView( flatView );
  }
}

void TodoView::onRowsInserted( const QModelIndex &parent, int start, int end)
{
    if ( start != end || !calendar() || !calendar()->entityTreeModel() )
        return;

    QModelIndex idx = mView->model()->index( start, 0 );

    // If the collection is currently being populated, we don't do anything
    QVariant v = idx.data( Akonadi::EntityTreeModel::ItemRole );
    if ( !v.isValid() )
        return;

    Akonadi::Item item = v.value<Akonadi::Item>();
    if ( !item.isValid() )
        return;

    const bool isPopulated = calendar()->entityTreeModel()->isCollectionPopulated( item.storageCollectionId() );
    if ( !isPopulated )
        return;

    // Case #1, adding an item that doesn't have parent: We select it
    if ( !parent.isValid() ) {
        QModelIndexList selection = mView->selectionModel()->selectedRows();
        if ( selection.size() <= 1 ) {
          // don't destroy complex selections, not applicable now (only single
          // selection allowed), but for the future...
          int colCount = static_cast<int>( TodoModel::ColumnCount );
          mView->selectionModel()->select( QItemSelection( idx, mView->model()->index( start, colCount-1 ) ),
                                           QItemSelectionModel::ClearAndSelect |
                                           QItemSelectionModel::Rows );
        }
        return;
    }

    // Case 2: Adding an item that has a parent: we expand the parent
    if ( sModels->isFlatView() )
        return;

    QModelIndex index = parent;
    mView->expand( index );
    while ( index.parent().isValid() ) {
      mView->expand( index.parent() );
      index = index.parent();
    }
}

void TodoView::getHighlightMode( bool &highlightEvents,
                                 bool &highlightTodos,
                                 bool &highlightJournals )
{
  highlightTodos    = preferences()->highlightTodos();
  highlightEvents   = !highlightTodos;
  highlightJournals = false;
}

bool TodoView::usesFullWindow()
{
  return preferences()->fullViewTodo();
}

void TodoView::resizeColumns()
{
  mResizeColumnsScheduled = false;

  mView->resizeColumnToContents( TodoModel::StartDateColumn );
  mView->resizeColumnToContents( TodoModel::DueDateColumn );
  mView->resizeColumnToContents( TodoModel::PriorityColumn);
  mView->resizeColumnToContents( TodoModel::CalendarColumn);
  mView->resizeColumnToContents( TodoModel::RecurColumn);
  mView->resizeColumnToContents( TodoModel::PercentColumn);

  // We have 3 columns that should stretch: summary, description and categories.
  // Summary is always visible.
  const bool descriptionVisible = !mView->isColumnHidden(TodoModel::DescriptionColumn);
  const bool categoriesVisible  = !mView->isColumnHidden(TodoModel::CategoriesColumn);

  // Calculate size of non-stretchable columns:
  int size = 0;
  for ( int i=0; i<TodoModel::ColumnCount; i++ ) {
    if ( !mView->isColumnHidden(i) && i != TodoModel::SummaryColumn && i != TodoModel::DescriptionColumn && i != TodoModel::CategoriesColumn )
      size += mView->columnWidth(i);
  }

  // Calculate the remaining space that we have for the stretchable columns
  int remainingSize = mView->header()->width() - size;

  // 100 for summary, 100 for description
  const int requiredSize = descriptionVisible ? 200 : 100;

  if ( categoriesVisible ) {
    const int categorySize = 100;
    mView->setColumnWidth( TodoModel::CategoriesColumn, categorySize );
    remainingSize -= categorySize;
  }

  if ( remainingSize < requiredSize ) {
    // We have too little size ( that's what she...), so lets use an horizontal scrollbar and make these columns use whatever they need.
    mView->resizeColumnToContents( TodoModel::SummaryColumn );
    mView->resizeColumnToContents( TodoModel::DescriptionColumn );
  } else if ( descriptionVisible ) {
    mView->setColumnWidth( TodoModel::SummaryColumn, remainingSize / 2 );
    mView->setColumnWidth( TodoModel::DescriptionColumn, remainingSize / 2 );
  } else {
    mView->setColumnWidth( TodoModel::SummaryColumn, remainingSize );
  }
}

void TodoView::restoreViewState()
{
  if ( sModels->isFlatView() ) {
    return;
  }

  if ( sModels->todoTreeModel && !sModels->todoTreeModel->sourceModel() ) {
    return;
  }

  //QElapsedTimer timer;
  //timer.start();
  delete mTreeStateRestorer;
  mTreeStateRestorer = new Akonadi::ETMViewStateSaver();
  KConfigGroup group( KGlobal::activeComponent().config().data(), stateSaverGroup() );
  mTreeStateRestorer->setView( mView );
  mTreeStateRestorer->restoreState( group );
  //qDebug() << "Took " << timer.elapsed();
}

QString TodoView::stateSaverGroup() const
{
  QString str = QLatin1String( "TodoTreeViewState" );
  if ( mSidebarView ) {
    str += QLatin1Char( 'S' );
  }

  return str;
}

void TodoView::saveViewState()
{
  Akonadi::ETMViewStateSaver treeStateSaver;
  KConfigGroup group( preferences()->config(), stateSaverGroup() );
  treeStateSaver.setView( mView );
  treeStateSaver.saveState( group );
}

void TodoView::resizeEvent( QResizeEvent *event )
{
  EventViews::EventView::resizeEvent( event );
  scheduleResizeColumns();
}

