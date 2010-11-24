/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Bertjan Broeksema <broeksema@kde.org>
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301  USA
*/

#include "mainview.h"

#include "calendar/incidenceview.h"
#include "calendar/kcalitembrowseritem.h"
#include "configwidget.h"
#include "searchwidget.h"
#include "settings.h"
#include "tasklistproxy.h"
#include "tasksactionmanager.h"
#include "tasksfilterproxymodel.h"
#include "tasksexporthandler.h"
#include "tasksimporthandler.h"

#include <incidenceeditor-ng/incidencedefaults.h>
#include <calendarsupport/archivedialog.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarutils.h>
#include <calendarsupport/freebusymanager.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/calendar/standardcalendaractionmanager.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcalcore/todo.h>
#include <KDebug>
#include <KGlobal>
#include <klocale.h>
#include <KStandardDirs>
#include <KSystemTimeZones>
#include <KMessageBox>
#include <libkdepimdbusinterfaces/reminderclient.h>

#include <QtCore/QPointer>
#include <QtDeclarative/QDeclarativeEngine>

using namespace Akonadi;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "tasks", new TaskListProxy, parent )
  , mCalendarUtils( 0 )
  , mTasksActionManager( 0 )
  , mCalendarPrefs( new EventViews::Prefs )
  , mCalendar( 0 )
  , mChanger( 0 )
{
  mCalendarPrefs->readConfig();
  qobject_cast<TaskListProxy*>( itemModel() )->setPreferences( mCalendarPrefs );

  // re-sort the list when config options have changed
  connect( Settings::self(), SIGNAL( configChanged() ),
           qobject_cast<TaskListProxy*>( itemModel() ), SLOT( invalidate() ) );
}

MainView::~MainView()
{
  mCalendarPrefs->writeConfig();
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "Tasks" ) );

  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.tasks", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.tasks", 4, 5, "SearchWidget" );

  mCalendar = new CalendarSupport::Calendar( entityTreeModel(), itemModel(),
                                             KSystemTimeZones::local() );

  mChanger = new CalendarSupport::IncidenceChanger( mCalendar, this );

  CalendarSupport::FreeBusyManager::self()->setCalendar( mCalendar );

  mCalendarUtils = new CalendarSupport::CalendarUtils( mCalendar, this );
  mCalendar->setParent( mCalendarUtils );
  connect( mCalendarUtils, SIGNAL( actionFinished( Akonadi::Item ) ),
          SLOT( processActionFinish( Akonadi::Item ) ) );
  connect( mCalendarUtils, SIGNAL( actionFailed( Akonadi::Item, QString ) ),
          SLOT( processActionFail( Akonadi::Item, QString ) ) );

  mTasksActionManager = new TasksActionManager( actionCollection(), this );
  mTasksActionManager->setCalendar( mCalendar );
  mTasksActionManager->setItemSelectionModel( itemSelectionModel() );

  connect( entityTreeModel(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
           mTasksActionManager, SLOT( updateActions() ) );

  connect( actionCollection()->action( QLatin1String( "import_tasks" ) ),
           SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  connect( actionCollection()->action( QLatin1String( "export_tasks" ) ),
           SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  connect( actionCollection()->action( QLatin1String( "make_subtask_independent" ) ),
           SIGNAL( triggered( bool ) ), SLOT( makeTaskIndependent() ) );
  connect( actionCollection()->action( QLatin1String( "make_all_subtasks_independent" ) ),
           SIGNAL( triggered( bool ) ), SLOT( makeAllSubtasksIndependent() ) );
  connect( actionCollection()->action( QLatin1String( "purge_completed_tasks" ) ),
           SIGNAL( triggered( bool ) ), SLOT( purgeCompletedTasks() ) );
  connect( actionCollection()->action( QLatin1String( "save_all_attachments" ) ),
           SIGNAL( triggered( bool ) ), SLOT( saveAllAttachments() ) );
  connect( actionCollection()->action( QLatin1String( "archive_old_entries" ) ),
           SIGNAL( triggered( bool ) ), SLOT( archiveOldEntries() ) );

  KPIM::ReminderClient::startDaemon();
}

void MainView::setConfigWidget( ConfigWidget *configWidget )
{
  Q_ASSERT( configWidget );
  if ( configWidget )
    configWidget->setPreferences( mCalendarPrefs );
}

void MainView::finishEdit( QObject *editor )
{
  mOpenItemEditors.remove( editor );
}

void MainView::newTask()
{
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Tasks" ) );

  Item item;
  item.setMimeType( KCalCore::Todo::todoMimeType() );
  KCalCore::Todo::Ptr todo( new KCalCore::Todo );

  { // Set some defaults
    IncidenceEditorNG::IncidenceDefaults defaults;
    // Set the full emails manually here, to avoid that we get dependencies on
    // KCalPrefs all over the place.
    defaults.setFullEmails( CalendarSupport::KCalPrefs::instance()->fullEmails() );
    // NOTE: At some point this should be generalized. That is, we now use the
    //       freebusy url as a hack, but this assumes that the user has only one
    //       groupware account. Which doesn't have to be the case necessarily.
    //       This method should somehow depend on the calendar selected to which
    //       the incidence is added.
    if ( CalendarSupport::KCalPrefs::instance()->useGroupwareCommunication() )
      defaults.setGroupWareDomain( KUrl( CalendarSupport::KCalPrefs::instance()->freeBusyRetrieveUrl() ).host() );

    // make it due one day from now
    const KDateTime now = KDateTime::currentLocalDateTime();
    defaults.setStartDateTime( now );
    defaults.setEndDateTime( now.addDays( 1 ) );

    defaults.setDefaults( todo );
  }

  item.setPayload<KCalCore::Todo::Ptr>( todo );
  editor->load( item );
  editor->show();
}

void MainView::newSubTask()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  KCalCore::Todo::Ptr parentTodo = item.payload<KCalCore::Todo::Ptr>();
  KCalCore::Todo::Ptr todo( new KCalCore::Todo );

  // make it due one day from now
  todo->setDtStart( KDateTime::currentLocalDateTime() );
  todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );
  todo->setRelatedTo( parentTodo->uid(), KCalCore::Todo::RelTypeParent );

  item.setPayload<KCalCore::Todo::Ptr>( todo );
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Tasks" ) );
  editor->load( item );
  editor->show();
}

void MainView::makeTaskIndependent()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  if ( mCalendarUtils->makeIndependent( item ) ) {
    actionCollection()->action( QLatin1String( "make_subtask_independent" ) )->setEnabled( false );
  }
}

void MainView::makeAllSubtasksIndependent()
{
  Item item = currentItem();
  if ( !item.isValid() )
    return;

  if ( mCalendarUtils->makeChildrenIndependent( item ) ) {
    actionCollection()->action( QLatin1String( "make_all_subtasks_independent" ) )->setEnabled( false );
  }
}

void MainView::purgeCompletedTasks()
{
  const int result = KMessageBox::warningContinueCancel(
    this,
    i18n( "Delete all completed to-dos?" ),
    i18n( "Purge To-dos" ),
    KGuiItem( i18n( "Purge" ) ) );

  if ( result == KMessageBox::Continue ) {
    mCalendarUtils->purgeCompletedTodos();
  }
}

void MainView::setPercentComplete( int row, int percentComplete )
{
  const QModelIndex index = itemModel()->index( row, 0 );
  itemModel()->setData( index, percentComplete, TaskListProxy::PercentComplete );
}

void MainView::editIncidence()
{
  const CalendarSupport::KCal::KCalItemBrowserItem *todoView = rootObject()->findChild<CalendarSupport::KCal::KCalItemBrowserItem*>();
  Q_ASSERT( todoView );
  if ( todoView )
    editIncidence( todoView->item() );
}

void MainView::editIncidence( const Akonadi::Item &item )
{
  if ( mOpenItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Tasks" ) );
  editor->load( item, QDate() );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL( destroyed( QObject* ) ), SLOT( finishEdit( QObject* ) ) );

  editor->show();
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  mStandardActionManager = new Akonadi::StandardCalendarActionManager( actionCollection(), this );
  mStandardActionManager->setCollectionSelectionModel( collectionSelectionModel );
  mStandardActionManager->setItemSelectionModel( itemSelectionModel );

  mStandardActionManager->createAllActions();
  mStandardActionManager->interceptAction( Akonadi::StandardActionManager::CreateResource );
  mStandardActionManager->interceptAction( Akonadi::StandardCalendarActionManager::CreateTodo );
  mStandardActionManager->interceptAction( Akonadi::StandardCalendarActionManager::CreateSubTodo );
  mStandardActionManager->interceptAction( Akonadi::StandardCalendarActionManager::EditIncidence );

  connect( mStandardActionManager->action( Akonadi::StandardActionManager::CreateResource ),
           SIGNAL( triggered( bool ) ), SLOT( launchAccountWizard() ) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo ),
           SIGNAL( triggered( bool ) ), SLOT( newTask() ) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateSubTodo ),
           SIGNAL( triggered( bool ) ), SLOT( newSubTask() ) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence ),
           SIGNAL( triggered( bool ) ), SLOT( editIncidence() ) );

  mStandardActionManager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Synchronize todos\nin account", "Synchronize todos\nin accounts" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  mStandardActionManager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  mStandardActionManager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Synchronize todos\nin folder", "Synchronize todos\nin folders" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  mStandardActionManager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete todo", "Delete todos" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::MoveItemToMenu )->setText( i18n( "Move todo\nto folder" ) );
  mStandardActionManager->action( Akonadi::StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy todo\nto folder" ) );
  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo )->setText( i18n( "New Task" ) );
  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateSubTodo )->setText( i18n( "New Sub Task" ) );
  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence )->setText( i18n( "Edit task" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize\nall todos" ) );
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
  return new TasksFilterProxyModel();
}

ImportHandlerBase* MainView::importHandler() const
{
  return new TasksImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new TasksExportHandler();
}

Item MainView::currentItem() const
{
  const QModelIndexList list = itemSelectionModel()->selectedRows();

  if ( list.size() != 1 )
    return Item();

  const QModelIndex index = list.first();
  const Item item = index.data( EntityTreeModel::ItemRole ).value<Item>();
  if ( !item.hasPayload<KCalCore::Todo::Ptr>() )
    return Item();

  return item;
}

void MainView::saveAllAttachments()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchForSaveAllAttachmentsDone( KJob* ) ) );
}

void MainView::fetchForSaveAllAttachmentsDone( KJob* job )
{
  if ( job->error() ) {
      kDebug() << "Error trying to fetch item";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n( "Cannot fetch calendar item." ),
                          i18n( "Item Fetch Error" ) );
      return;
  }

  const Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();

  CalendarSupport::saveAttachments( item, this );
}

void MainView::archiveOldEntries()
{
  CalendarSupport::ArchiveDialog archiveDialog( mCalendar, mChanger, this );
  archiveDialog.exec();
}

void MainView::processActionFail( const Akonadi::Item &item, const QString &msg )
{
  Q_UNUSED( item );
  Q_UNUSED( msg );
  mTasksActionManager->updateActions();
}

void MainView::processActionFinish( const Akonadi::Item &item )
{
  Q_UNUSED( item );
  mTasksActionManager->updateActions();
}

