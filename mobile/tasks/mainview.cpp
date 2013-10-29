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

#include "actionhelper.h"
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
#include "taskthreadgroupercomparator.h"
#include "threadgroupermodel.h"

#include <incidenceeditor-ng/categoryeditdialog.h>
#include <incidenceeditor-ng/editorconfig.h>
#include <incidenceeditor-ng/incidencedefaults.h>
#include <calendarsupport/archivedialog.h>
#include <calendarsupport/calendarutils.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/calendar/standardcalendaractionmanager.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <akonadi/calendar/freebusymanager.h>
#include <akonadi/calendar/calendarsettings.h>

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
#include <QDeclarativeEngine>

using namespace Akonadi;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )

MainView::MainView( QWidget *parent )
  : KDeclarativeMainView( QLatin1String("tasks"), new TaskListProxy, parent )
  , mCalendarUtils( 0 )
  , mTasksActionManager( 0 )
  , mCalendarPrefs( new EventViews::Prefs )
  , mCalendar( 0 )
  , mChanger( 0 )
{
  mCalendarPrefs->readConfig();
  qobject_cast<TaskListProxy*>( itemModel() )->setPreferences( mCalendarPrefs );

  // re-sort the list when config options have changed
  connect( Settings::self(), SIGNAL(configChanged()),
           qobject_cast<TaskListProxy*>( itemModel() ), SLOT(invalidate()) );
}

MainView::~MainView()
{
  mCalendarPrefs->writeConfig();
}

void MainView::doDelayedInit()
{
  setWindowTitle( i18n( "Tasks" ) );

  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.tasks", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.tasks", 4, 5, "SearchWidget" );

  QStringList mimeTypes;
  mimeTypes << KCalCore::Todo::todoMimeType();
  mCalendar = Akonadi::ETMCalendar::Ptr( new Akonadi::ETMCalendar( mimeTypes ) );

  mChanger = new Akonadi::IncidenceChanger( this );

  Akonadi::FreeBusyManager::self()->setCalendar( mCalendar );

  mCalendarUtils = new CalendarSupport::CalendarUtils( mCalendar, this );
  mCalendar->setParent( mCalendarUtils );
  connect( mCalendarUtils, SIGNAL(actionFinished(Akonadi::Item)),
          SLOT(processActionFinish(Akonadi::Item)) );
  connect( mCalendarUtils, SIGNAL(actionFailed(Akonadi::Item,QString)),
          SLOT(processActionFail(Akonadi::Item,QString)) );

  mTasksActionManager = new TasksActionManager( actionCollection(), this );
  mTasksActionManager->setCalendar( mCalendar );
  mTasksActionManager->setItemSelectionModel( itemSelectionModel() );

  connect( entityTreeModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           mTasksActionManager, SLOT(updateActions()) );

  connect( actionCollection()->action( QLatin1String( "import_tasks" ) ),
           SIGNAL(triggered(bool)), SLOT(importItems()) );
  connect( actionCollection()->action( QLatin1String( "export_account_tasks" ) ),
           SIGNAL(triggered(bool)), SLOT(exportItems()) );
  connect( actionCollection()->action( QLatin1String( "export_selected_tasks" ) ),
           SIGNAL(triggered(bool)), SLOT(exportItems()) );
  connect( actionCollection()->action( QLatin1String( "make_subtask_independent" ) ),
           SIGNAL(triggered(bool)), SLOT(makeTaskIndependent()) );
  connect( actionCollection()->action( QLatin1String( "make_all_subtasks_independent" ) ),
           SIGNAL(triggered(bool)), SLOT(makeAllSubtasksIndependent()) );
  connect( actionCollection()->action( QLatin1String( "purge_completed_tasks" ) ),
           SIGNAL(triggered(bool)), SLOT(purgeCompletedTasks()) );
  connect( actionCollection()->action( QLatin1String( "save_all_attachments" ) ),
           SIGNAL(triggered(bool)), SLOT(saveAllAttachments()) );
  connect( actionCollection()->action( QLatin1String( "archive_old_entries" ) ),
           SIGNAL(triggered(bool)), SLOT(archiveOldEntries()) );

  KAction *action = new KAction( i18n( "Configure Categories" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(configureCategories()) );
  actionCollection()->addAction( QLatin1String( "configure_categories" ), action );

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
  editor->setWindowTitle( i18n( "Kontact Touch Tasks" ) );

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
      defaults.setGroupWareDomain( KUrl( Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl() ).host() );

    // make it due one day from now
    const KDateTime now = KDateTime::currentLocalDateTime();
    defaults.setStartDateTime( now );
    defaults.setEndDateTime( now.addDays( 1 ) );

    defaults.setDefaults( todo );
  }

  item.setPayload<KCalCore::Todo::Ptr>( todo );
  editor->load( item );

  if ( regularSelectionModel()->hasSelection() ) {
    const QModelIndex index = regularSelectionModel()->selectedIndexes().first();
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      editor->setDefaultCollection( collection );
  }

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
  editor->setWindowTitle( i18n( "Kontact Touch Tasks" ) );
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
  itemModel()->setData( index, percentComplete, TaskListProxy::PercentCompleteRole );
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
  editor->setWindowTitle( i18n( "Kontact Touch Tasks" ) );
  editor->load( item, QDate() );

  mOpenItemEditors.insert(  editor, item.id() );
  connect( editor, SIGNAL(destroyed(QObject*)), SLOT(finishEdit(QObject*)) );

  editor->show();
}

QAbstractItemModel* MainView::createItemModelContext( QDeclarativeContext *context, QAbstractItemModel *model )
{
  TaskThreadGrouperComparator *comparator = new TaskThreadGrouperComparator;
  ThreadGrouperModel *grouperModel = new ThreadGrouperModel( comparator, this );
  grouperModel->setDynamicModelRepopulation( true );
  grouperModel->setSourceModel( model );

  // trigger a resort whenever the task status has changed
  connect( model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
           grouperModel, SLOT(invalidate()) );

  return KDeclarativeMainView::createItemModelContext( context, grouperModel );
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
           SIGNAL(triggered(bool)), SLOT(launchAccountWizard()) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo ),
           SIGNAL(triggered(bool)), SLOT(newTask()) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateSubTodo ),
           SIGNAL(triggered(bool)), SLOT(newSubTask()) );
  connect( mStandardActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence ),
           SIGNAL(triggered(bool)), SLOT(editIncidence()) );
  connect( mStandardActionManager, SIGNAL(actionStateUpdated()), SLOT(updateActionTexts()) );

  ActionHelper::adaptStandardActionTexts( mStandardActionManager );

  mStandardActionManager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Task List Properties" ) );
  mStandardActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Task List" ) );
  mStandardActionManager->action( StandardActionManager::CreateCollection )->setProperty( "ContentMimeTypes", QStringList( KCalCore::Todo::todoMimeType() ) );
  mStandardActionManager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize This Task List", "Synchronize These Task Lists" ) );
  mStandardActionManager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete Task List", "Delete Task Lists" ) );
  mStandardActionManager->action( StandardActionManager::MoveCollectionToDialog )->setText( i18n( "Move Task List To" ) );
  mStandardActionManager->action( StandardActionManager::CopyCollectionToDialog )->setText( i18n( "Copy Task List To" ) );

  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo )->setText( i18n( "New Task" ) );
  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::CreateSubTodo )->setText( i18n( "New Sub Task" ) );
  mStandardActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence )->setText( i18n( "Edit task" ) );

  actionCollection()->action( QLatin1String("synchronize_all_items") )->setText( i18n( "Synchronize All Tasks" ) );
}

void MainView::updateActionTexts()
{
  const Akonadi::Item::List items = mStandardActionManager->selectedItems();
  if ( items.count() < 1 )
    return;

  const int itemCount = items.count();
  const Akonadi::Item item = items.first();
  const QString mimeType = item.mimeType();
  if ( mimeType == KCalCore::Event::eventMimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Event", "Copy %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Event To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Event", "Delete %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Event To" ) );
    actionCollection()->action( QLatin1String("akonadi_incidence_edit") )->setText( i18n( "Edit Event" ) );
  } else if ( mimeType == KCalCore::Todo::todoMimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Task", "Copy %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Task To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Task", "Delete %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Task To" ) );
    actionCollection()->action( QLatin1String("akonadi_incidence_edit") )->setText( i18n( "Edit Task" ) );
  } else if ( mimeType == KCalCore::Journal::journalMimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Journal", "Copy %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Journal To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Journal", "Delete %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Journal To" ) );
    actionCollection()->action( QLatin1String("akonadi_incidence_edit") )->setText( i18n( "Edit Journal" ) );
  }
}

void MainView::setupAgentActionManager( QItemSelectionModel *selectionModel )
{
  Akonadi::AgentActionManager *manager = createAgentActionManager( selectionModel );

  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::DialogTitle,
                           i18nc( "@title:window", "New Account" ) );
  manager->setContextText( Akonadi::AgentActionManager::CreateAgentInstance, Akonadi::AgentActionManager::ErrorMessageText,
                           ki18n( "Could not create account: %1" ) );
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

void MainView::configureCategories()
{
  CalendarSupport::CategoryConfig config( IncidenceEditorNG::EditorConfig::instance()->config(), 0 );
  IncidenceEditorNG::CategoryEditDialog dialog( &config, 0 );
  if ( dialog.exec() )
    config.writeConfig();
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
  connect( job, SIGNAL(result(KJob*)), this, SLOT(fetchForSaveAllAttachmentsDone(KJob*)) );
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
#ifndef Q_OS_WINCE
  CalendarSupport::saveAttachments( item, this );
#else
  // CalendarSupport is not completely ported for Windows CE so we use the
  // attachment handling code from KDeclarativeMainView
  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );

  if ( !incidence ) {
    KMessageBox::sorry(
      this,
      i18n( "No item selected." ),
      QLatin1String("SaveAttachments") );
    return;
  }

  KCalCore::Attachment::List attachments = incidence->attachments();

  if ( attachments.empty() )
    return;

  Q_FOREACH( KCalCore::Attachment::Ptr attachment, attachments ) {
    QString fileName = attachment->label();
    QString sourceUrl;
    if ( attachment->isUri() ) {
      sourceUrl = attachment->uri();
    } else {
      sourceUrl = incidence->writeAttachmentToTempFile( attachment );
    }
      saveAttachment( sourceUrl, fileName );
  }
#endif //Q_OS_WINCE
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

