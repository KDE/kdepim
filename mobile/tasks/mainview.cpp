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
#include "tasklistproxy.h"
#include "tasksactionmanager.h"
#include "tasksfilterproxymodel.h"
#include "tasksexporthandler.h"
#include "tasksimporthandler.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarutils.h>
#include <calendarsupport/freebusymanager.h>
#include <calendarsupport/utils.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemmodifyjob.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/standardactionmanager.h>

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

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "tasks", new TaskListProxy, parent )
  , mCalendarUtils( 0 )
  , mTasksActionManager( 0 )
{
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "KDE Tasks" ) );

  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );

  CalendarSupport::Calendar *cal = new CalendarSupport::Calendar( entityTreeModel(),
                                                                  regularSelectedItems(),
                                                                  KSystemTimeZones::local() );
  CalendarSupport::FreeBusyManager::self()->setCalendar( cal );

  mCalendarUtils = new CalendarSupport::CalendarUtils( cal, this );
  cal->setParent( mCalendarUtils );
  connect( mCalendarUtils, SIGNAL( actionFinished( Akonadi::Item ) ),
          SLOT( processActionFinish( Akonadi::Item ) ) );
  connect( mCalendarUtils, SIGNAL( actionFailed( Akonadi::Item, QString ) ),
          SLOT( processActionFail( Akonadi::Item, QString ) ) );

  mTasksActionManager = new TasksActionManager( actionCollection(), this );
  mTasksActionManager->setCalendar( cal );
  mTasksActionManager->setItemSelectionModel( itemSelectionModel() );

  connect( entityTreeModel(), SIGNAL( dataChanged( QModelIndex, QModelIndex ) ),
           mTasksActionManager, SLOT( updateActions() ) );

  connect( actionCollection()->action( QLatin1String( "add_new_task" ) ),
           SIGNAL( triggered( bool ) ), SLOT( newTask() ) );
  connect( actionCollection()->action( QLatin1String( "add_new_subtask" ) ),
           SIGNAL( triggered( bool ) ), SLOT( newSubTask() ) );
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

  KPIM::ReminderClient::startDaemon();
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

  // make it due one day from now
  todo->setDtStart( KDateTime::currentLocalDateTime() );
  todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );

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

}

void MainView::setPercentComplete( int row, int percentComplete )
{
  const QModelIndex index = itemModel()->index( row, 0 );
  itemModel()->setData( index, percentComplete, TaskListProxy::PercentComplete );
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
  Akonadi::StandardActionManager *manager = new Akonadi::StandardActionManager( actionCollection(), this );
  manager->setCollectionSelectionModel( collectionSelectionModel );
  manager->setItemSelectionModel( itemSelectionModel );

  manager->createAllActions();
  manager->interceptAction( Akonadi::StandardActionManager::CreateResource );

  connect( manager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );

  manager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Synchronize todos\nin account", "Synchronize todos\nin accounts" ) );
  manager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  manager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  manager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Synchronize todos\nin folder", "Synchronize todos\nin folders" ) );
  manager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  manager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  manager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete todo", "Delete todos" ) );
  manager->action( Akonadi::StandardActionManager::MoveItemToMenu )->setText( i18n( "Move todo\nto folder" ) );
  manager->action( Akonadi::StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy todo\nto folder" ) );

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

QAbstractProxyModel* MainView::itemFilterModel() const
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

