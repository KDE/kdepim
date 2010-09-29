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

#include <kaction.h>
#include <kactioncollection.h>
#include <kcalcore/filestorage.h>
#include <kcalcore/icalformat.h>
#include <kcalcore/memorycalendar.h>
#include <kcalcore/todo.h>
#include <KDebug>
#include <kfiledialog.h>
#include <KGlobal>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprogressdialog.h>
#include <KStandardDirs>

#include <akonadi/agentactionmanager.h>
#include <akonadi/collectiondialog.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemcreatejob.h>
#include <akonadi/itemfetchjob.h>
#include <Akonadi/ItemFetchScope>
#include <akonadi/recursiveitemfetchjob.h>
#include <akonadi/standardactionmanager.h>
#include <libkdepimdbusinterfaces/reminderclient.h>

#include "calendar/incidenceview.h"
#include "calendar/kcalitembrowseritem.h"
#include "tasklistproxy.h"
#include "tasksfilterproxymodel.h"

#include <QtCore/QPointer>
#include <QtDeclarative/QDeclarativeEngine>

using namespace Akonadi;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( "tasks", new TaskListProxy, parent ),
    m_importProgressDialog( 0 )
{
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "KOrganizer Task Manager" ) );

  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );

  KAction *action = new KAction( i18n( "New Task" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newTask()) );
  actionCollection()->addAction( QLatin1String( "add_new_task" ), action );

  action = new KAction( i18n( "Import Tasks" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importICal() ) );
  actionCollection()->addAction( QLatin1String( "import_tasks" ), action );

  action = new KAction( i18n( "Export Tasks" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportICal() ) );
  actionCollection()->addAction( QLatin1String( "export_tasks" ), action );

  KPIM::ReminderClient::startDaemon();
}

void MainView::newTask()
{
  IncidenceView *editor = new IncidenceView;
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

void MainView::setPercentComplete(int row, int percentComplete)
{
  const QModelIndex idx = itemModel()->index(row, 0);
  itemModel()->setData(idx, percentComplete, TaskListProxy::PercentComplete);
}

void MainView::editIncidence( const Akonadi::Item &item )
{
  IncidenceView *editor = new IncidenceView;
  editor->load( item, QDate() );
  editor->show();
}

void MainView::importICal()
{
  const QStringList fileNames = KFileDialog::getOpenFileNames( KUrl(), "*.ics|iCal", 0,
                                                               i18n( "Select iCal to Import" ) );

  if ( fileNames.count() == 0 )
    return;

  bool anyFailures = false;

  KCalCore::Todo::List todos;

  foreach ( const QString &fileName, fileNames ) {
    KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );

    KCalCore::FileStorage::Ptr storage( new KCalCore::FileStorage( calendar, fileName, new KCalCore::ICalFormat() ) );

    if ( storage->load() ) {
      todos << calendar->todos();
    } else {
      const QString caption( i18n( "iCal Import Failed" ) );
      const QString msg = i18nc( "@info",
                                 "<para>Error when trying to read the iCal <filename>%1</filename>:</para>",
                                 fileName );
      KMessageBox::error( 0, msg, caption );
      anyFailures = true;
    }
  }

  if ( todos.isEmpty() ) {
    if ( anyFailures && fileNames.count() > 1 )
      KMessageBox::information( 0, i18n( "No tasks were imported, due to errors with the iCals." ) );
    else if ( !anyFailures )
      KMessageBox::information( 0, i18n( "The iCal does not contain any tasks." ) );

    return; // nothing to import
  }

  const QStringList mimeTypes( KCalCore::Todo::todoMimeType() );

  QPointer<Akonadi::CollectionDialog> dlg = new Akonadi::CollectionDialog();
  dlg->setMimeTypeFilter( mimeTypes );
  dlg->setAccessRightsFilter( Akonadi::Collection::CanCreateItem );
  dlg->setCaption( i18n( "Select Calendar" ) );
  dlg->setDescription( i18n( "Select the calendar the imported todo(s) shall be saved in:" ) );

  // preselect the currently selected folder
  const QModelIndexList indexes = regularSelectionModel()->selectedRows();
  if ( !indexes.isEmpty() ) {
    const QModelIndex collectionIndex = indexes.first();
    const Akonadi::Collection collection = collectionIndex.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      dlg->setDefaultCollection( collection );
  }

  if ( !dlg->exec() || !dlg ) {
    delete dlg;
    return;
  }

  const Akonadi::Collection collection = dlg->selectedCollection();
  delete dlg;

  if ( !m_importProgressDialog ) {
    m_importProgressDialog = new KProgressDialog( 0, i18n( "Import Todos" ) );
    m_importProgressDialog->setLabelText( i18np( "Importing one todo to %2", "Importing %1 todos to %2",
                                                 todos.count(), collection.name() ) );
    m_importProgressDialog->setAllowCancel( false );
    m_importProgressDialog->setAutoClose( true );
    m_importProgressDialog->progressBar()->setRange( 1, todos.count() );
  }

  m_importProgressDialog->show();

  foreach ( const KCalCore::Todo::Ptr &todo, todos ) {
    Akonadi::Item item;
    item.setPayload<KCalCore::Todo::Ptr>( todo );
    item.setMimeType( KCalCore::Todo::todoMimeType() );

    Akonadi::ItemCreateJob *job = new Akonadi::ItemCreateJob( item, collection );
    connect( job, SIGNAL( result( KJob* ) ), SLOT( slotImportJobDone( KJob* ) ) );
  }
}

void MainView::slotImportJobDone( KJob* )
{
  if ( !m_importProgressDialog )
    return;

  QProgressBar *progressBar = m_importProgressDialog->progressBar();

  progressBar->setValue( progressBar->value() + 1 );

  // cleanup on last step
  if ( progressBar->value() == progressBar->maximum() ) {
    m_importProgressDialog->deleteLater();
    m_importProgressDialog = 0;
  }
}

void MainView::exportICal()
{
  Akonadi::Collection::List selectedCollections;
  const QModelIndexList indexes = regularSelectionModel()->selectedRows();
  foreach ( const QModelIndex &index, indexes ) {
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      selectedCollections << collection;
  }

  bool exportAllTodos = false;
  if ( !selectedCollections.isEmpty() ) {
    const QString msg = i18n( "Which todos shall be exported?" );
    switch ( KMessageBox::questionYesNo( 0, msg, QString(), KGuiItem(i18n( "All Todos" ) ),
                                         KGuiItem( i18n( "Todos in current folder" ) ) ) ) {
      case KMessageBox::Yes:
        exportAllTodos = true;
        break;
      case KMessageBox::No: // fall through
      default:
        exportAllTodos = false;
    }
  } else {
    exportAllTodos = true;
  }

  Akonadi::Item::List todoItems;
  if ( exportAllTodos ) {
    Akonadi::RecursiveItemFetchJob *job = new Akonadi::RecursiveItemFetchJob( Akonadi::Collection::root(),
                                                                              QStringList() << KCalCore::Todo::todoMimeType() );
    job->fetchScope().fetchFullPayload();

    job->exec();

    todoItems << job->items();
  } else {
    foreach ( const Akonadi::Collection &collection, selectedCollections ) {
      Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( collection );
      job->fetchScope().fetchFullPayload();

      if ( job->exec() )
        todoItems << job->items();
    }
  }

  KCalCore::Todo::List todos;

  foreach ( const Akonadi::Item &item, todoItems ) {
    if ( item.hasPayload<KCalCore::Todo::Ptr>() )
      todos << item.payload<KCalCore::Todo::Ptr>();
  }

  if ( todos.isEmpty() )
    return;

  const QString fileName = KFileDialog::getSaveFileName( KUrl( "calendar.ics" ) );
  if ( fileName.isEmpty() )
    return;

  KCalCore::MemoryCalendar::Ptr calendar( new KCalCore::MemoryCalendar( QLatin1String( "UTC" ) ) );
  calendar->startBatchAdding();
  foreach ( const KCalCore::Todo::Ptr &todo, todos )
    calendar->addIncidence( todo );
  calendar->endBatchAdding();

  KCalCore::FileStorage::Ptr storage( new KCalCore::FileStorage( calendar, fileName, new KCalCore::ICalFormat() ) );

  if ( storage->open() ) {
    storage->save();
    storage->close();
  }
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

