/*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
* Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Copyright (c) 2010 Andras Mantia <andras@kdab.com>
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
#include "agendaviewitem.h"
#include "calendaradaptor.h"
#include "calendarinterface.h"
#include "calendar/incidenceview.h"
#include "calendar/kcalitembrowseritem.h"
#include "configwidget.h"
#include "eventlistproxy.h"
#include "eventsexporthandler.h"
#include "eventsfilterproxymodel.h"
#include "eventsguistatemanager.h"
#include "eventsimporthandler.h"
#include "monthviewitem.h"
#include "searchwidget.h"
#include "timelineviewitem.h"
#include "qmldateedit.h"

#include <akonadi/agentactionmanager.h>
#include <akonadi/calendar/standardcalendaractionmanager.h>
#include <akonadi/collectionmodel.h>
#include <akonadi/collectionpropertiesdialog.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/standardactionmanager.h>
#include <calendarsupport/archivedialog.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectiongeneralpage.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/freebusymanager.h>
#include <calendarsupport/identitymanager.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <calendarviews/eventviews/eventview.h>
#include <calendarviews/eventviews/agenda/agendaview.h>
#include <calendarviews/eventviews/month/monthview.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <kcalcore/event.h>
#include <kcalcore/todo.h>
#ifndef _WIN32_WCE
#include <kcolordialog.h>
#endif
#include <kmessagebox.h>
#include <ksystemtimezone.h>
#include <incidenceeditor-ng/incidencedefaults.h>
#include <libkdepimdbusinterfaces/reminderclient.h>

#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtGui/QGraphicsItem>

Q_DECLARE_METATYPE(KCalCore::iTIPMethod)

using namespace Akonadi;
using CalendarSupport::KCalPrefs;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )
QML_DECLARE_TYPE( DeclarativeConfigWidget )
QML_DECLARE_TYPE( DeclarativeSearchWidget )
QML_DECLARE_TYPE( Qt::QmlDateEdit )
QML_DECLARE_TYPE( EventsGuiStateManager )
QML_DECLARE_TYPE( EventViews::AgendaViewItem )
QML_DECLARE_TYPE( EventViews::MonthViewItem )
QML_DECLARE_TYPE( EventViews::TimelineViewItem )

EventViews::PrefsPtr MainView::m_calendarPrefs;

MainView::MainView( QWidget* parent )
  : KDeclarativeMainView( "korganizer-mobile", new EventListProxy, parent ),
    m_calendar( 0 ),
    m_identityManager( 0 ),
    m_changer( 0 ),
    mActionManager( 0 )
{
  m_calendarPrefs = EventViews::PrefsPtr( new  EventViews::Prefs );
  m_calendarPrefs->readConfig();

  Akonadi::CollectionPropertiesDialog::registerPage( new CalendarSupport::CollectionGeneralPageFactory );
}

MainView::~MainView()
{
  m_calendarPrefs->writeConfig();
  m_calendar->deleteLater();
  delete m_identityManager; 
}

EventViews::PrefsPtr MainView::preferences()
{
  return m_calendarPrefs;
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "Calendar" ) );

  addMimeType( KCalCore::Event::eventMimeType() );
  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );
  qmlRegisterType<DeclarativeConfigWidget>( "org.kde.akonadi.calendar", 4, 5, "ConfigWidget" );
  qmlRegisterType<DeclarativeSearchWidget>( "org.kde.akonadi.calendar", 4, 5, "SearchWidget" );
  qmlRegisterType<EventViews::AgendaViewItem>( "org.kde.calendarviews", 4, 5, "AgendaView" );
  qmlRegisterType<EventViews::MonthViewItem>( "org.kde.calendarviews", 4, 5, "MonthView" );
  qmlRegisterType<EventViews::TimelineViewItem>( "org.kde.calendarviews", 4, 5, "TimelineView" );
  qmlRegisterType<Qt::QmlDateEdit>( "org.qt", 4, 7, "QmlDateEdit" );
  qmlRegisterUncreatableType<EventsGuiStateManager>( "org.kde.akonadi.events", 4, 5, "EventsGuiStateManager", QLatin1String( "This type is only exported for its enums" ) );

  m_calendar = new CalendarSupport::Calendar( entityTreeModel(), itemModel(), KSystemTimeZones::local() );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( m_calendar ) ) );
  CalendarSupport::FreeBusyManager::self()->setCalendar( m_calendar );

  m_changer = new CalendarSupport::IncidenceChanger( m_calendar, this );

  m_identityManager = new CalendarSupport::IdentityManager;

  // FIXME: My suspicion is that this is wrong. I.e. the collection selection is
  //        not correct resulting in no items showing up in the monthview.
  CalendarSupport::CollectionSelection *collectionselection;
  collectionselection = new CalendarSupport::CollectionSelection( regularSelectionModel(), this );
  EventViews::EventView::setGlobalCollectionSelection( collectionselection );

  QDBusConnection::sessionBus().registerService( "org.kde.korganizer" ); //register also as the real korganizer, so kmail can communicate with it

  KAction *action = new KAction( i18n( "Import Events" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  actionCollection()->addAction( QLatin1String( "import_events" ), action );

  action = new KAction( i18n( "Export Events From This Account" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_account_events" ), action );

  action = new KAction( i18n( "Export Displayed Events" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_selected_events" ), action );

  action = new KAction( i18n( "Archive Old Events" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( archiveOldEntries() ) );
  actionCollection()->addAction( QLatin1String( "archive_old_entries" ), action );

  action = new KAction( i18n( "Publish Item Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( publishItemInformation() ) );
  actionCollection()->addAction( QLatin1String( "publish_item_information" ), action );

  action = new KAction( i18n( "Send Invitations To Attendees" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendInvitation() ) );
  actionCollection()->addAction( QLatin1String( "send_invitations_to_attendees" ), action );

  action = new KAction( i18n( "Send Status Update" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendStatusUpdate() ) );
  actionCollection()->addAction( QLatin1String( "send_status_update" ), action );

  action = new KAction( i18n( "Send Cancellation To Attendees" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendCancellation() ) );
  actionCollection()->addAction( QLatin1String( "send_cancellation_to_attendees" ), action );

  action = new KAction( i18n( "Request Update" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( requestUpdate() ) );
  actionCollection()->addAction( QLatin1String( "request_update" ), action );

  action = new KAction( i18n( "Request Change" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( requestChange() ) );
  actionCollection()->addAction( QLatin1String( "request_change" ), action );

  action = new KAction( i18n( "Send As ICalendar" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendAsICalendar() ) );
  actionCollection()->addAction( QLatin1String( "send_as_icalendar" ), action );

  action = new KAction( i18n( "Mail Free Busy Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( mailFreeBusy() ) );
  actionCollection()->addAction( QLatin1String( "mail_freebusy" ), action );

  action = new KAction( i18n( "Upload Free Busy Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( uploadFreeBusy() ) );
  actionCollection()->addAction( QLatin1String( "upload_freebusy" ), action );

  action = new KAction( i18n( "Save All" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( saveAllAttachments() ) );
  actionCollection()->addAction( QLatin1String( "save_all_attachments" ), action );

  action = new KAction( i18n( "Set Color Of Calendar" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( changeCalendarColor()) );
  actionCollection()->addAction( QLatin1String( "set_calendar_colour" ), action );

  connect( this, SIGNAL( statusChanged( QDeclarativeView::Status ) ),
           this, SLOT( qmlLoadingStateChanged( QDeclarativeView::Status ) ) );

  //register DBUS interface
  m_calendarIface = new CalendarInterface( this );
  new CalendarAdaptor( m_calendarIface );
  QDBusConnection::sessionBus().registerObject( "/Calendar", m_calendarIface );

  KPIM::ReminderClient::startDaemon();
}

void MainView::qmlLoadingStateChanged( QDeclarativeView::Status status )
{
  if ( status != Ready ) // We wait until the QML is completely loaded
    return;

  connect( m_calendarIface, SIGNAL( showDateSignal( QVariant ) ), rootObject(), SLOT( showDate( QVariant ) ) );
  connect( m_calendarIface, SIGNAL( showEventViewSignal() ), rootObject(), SLOT( showEventView() ) );

  // setup the shared settings object
  EventViews::AgendaViewItem *agendaViewItem = rootObject()->findChild<EventViews::AgendaViewItem*>();
  Q_ASSERT( agendaViewItem );
  if ( agendaViewItem )
    agendaViewItem->setPreferences( m_calendarPrefs );
}

void MainView::setConfigWidget(ConfigWidget* configWidget)
{
  Q_ASSERT( configWidget );
  configWidget->setPreferences( m_calendarPrefs );

  EventViews::AgendaViewItem *agendaViewItem = rootObject()->findChild<EventViews::AgendaViewItem*>();
  if ( agendaViewItem )
    connect( configWidget, SIGNAL(configChanged()), agendaViewItem, SLOT(updateConfig()) );
  EventViews::MonthViewItem *monthViewItem = rootObject()->findChild<EventViews::MonthViewItem*>();
  if ( monthViewItem )
    connect( configWidget, SIGNAL(configChanged()), monthViewItem, SLOT(updateConfig()) );
}

void MainView::finishEdit( QObject *editor )
{
  m_openItemEditors.remove( editor );
}

void MainView::showRegularCalendar()
{
  m_calendar->setUnfilteredModel( itemModel() );
}

void MainView::setCurrentEventItemId( qint64 id )
{
  const QModelIndexList list = EntityTreeModel::modelIndexesForItem(itemSelectionModel()->model(), Item(id));
  if (list.isEmpty())
    return;

  const QModelIndex idx = list.first();
  itemSelectionModel()->select( QItemSelection(idx, idx), QItemSelectionModel::ClearAndSelect );
  itemActionModel()->select( QItemSelection( idx, idx ), QItemSelectionModel::ClearAndSelect );
}

void MainView::newEvent()
{
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Calendar" ) );
  Item item;
  item.setMimeType( KCalCore::Event::eventMimeType() );
  KCalCore::Event::Ptr event( new KCalCore::Event );

  IncidenceEditorNG::IncidenceDefaults defaults;
  // Set the full emails manually here, to avoid that we get dependencies on
  // KCalPrefs all over the place.
  defaults.setFullEmails( CalendarSupport::KCalPrefs::instance()->fullEmails() );
  // NOTE: At some point this should be generalized. That is, we now use the
  //       freebusy url as a hack, but this assumes that the user has only one
  //       groupware account. Which doesn't have to be the case necessarily.
  //       This method should somehow depend on the calendar selected to which
  //       the incidence is added.
  if ( KCalPrefs::instance()->useGroupwareCommunication() )
    defaults.setGroupWareDomain( KUrl( KCalPrefs::instance()->freeBusyRetrieveUrl() ).host() );

  defaults.setDefaults( event );

  item.setPayload<KCalCore::Event::Ptr>( event );
  editor->load( item );
  editor->show();
}

void MainView::newTodo()
{
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Calendar" ) );
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

void MainView::editIncidence()
{
  const CalendarSupport::KCal::KCalItemBrowserItem *eventView = rootObject()->findChild<CalendarSupport::KCal::KCalItemBrowserItem*>();
  Q_ASSERT( eventView );
  if ( eventView )
    editIncidence( eventView->item(), eventView->activeDate() );
}

void MainView::editIncidence( const Akonadi::Item &item, const QDate &date )
{
  if ( m_openItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "KDE Calendar" ) );
  editor->load( item, date );

  m_openItemEditors.insert( editor, item.id() );
  connect( editor, SIGNAL( destroyed( QObject* ) ), SLOT( finishEdit( QObject* ) ) );

  editor->show();
}

void MainView::deleteIncidence()
{
  const CalendarSupport::KCal::KCalItemBrowserItem *eventView = rootObject()->findChild<CalendarSupport::KCal::KCalItemBrowserItem*>();
  Q_ASSERT( eventView );
  if ( eventView )
    deleteIncidence( eventView->item() );
}

void MainView::deleteIncidence( const Akonadi::Item &item )
{
  m_changer->deleteIncidence( item );
}

void MainView::setupStandardActionManager( QItemSelectionModel *collectionSelectionModel,
                                           QItemSelectionModel *itemSelectionModel )
{
  mActionManager = new Akonadi::StandardCalendarActionManager( actionCollection(), this );
  mActionManager->setCollectionSelectionModel( collectionSelectionModel );
  mActionManager->setItemSelectionModel( itemSelectionModel );

  mActionManager->createAllActions();
  mActionManager->interceptAction( Akonadi::StandardActionManager::CreateResource );
  mActionManager->interceptAction( Akonadi::StandardActionManager::DeleteItems );
  mActionManager->interceptAction( Akonadi::StandardCalendarActionManager::CreateEvent );
  mActionManager->interceptAction( Akonadi::StandardCalendarActionManager::CreateTodo );
  mActionManager->interceptAction( Akonadi::StandardCalendarActionManager::EditIncidence );

  connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL( triggered( bool ) ),
           this, SLOT( launchAccountWizard() ) );
  connect( mActionManager->action( Akonadi::StandardActionManager::DeleteItems ), SIGNAL( triggered( bool ) ),
           this, SLOT( deleteIncidence() ) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::CreateEvent ), SIGNAL( triggered( bool ) ),
           this, SLOT( newEvent() ) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo ), SIGNAL( triggered( bool ) ),
           this, SLOT( newTodo() ) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence ), SIGNAL( triggered( bool ) ),
           this, SLOT( editIncidence() ) );
  connect( mActionManager, SIGNAL( actionStateUpdated() ), SLOT( updateActionTexts() ) );

  ActionHelper::adaptStandardActionTexts( mActionManager );

  mActionManager->action( Akonadi::StandardCalendarActionManager::CreateEvent )->setText( i18n( "New Event" ) );
  mActionManager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Calendar Properties" ) );
  mActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Calendar" ) );
  mActionManager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize This Calendar", "Synchronize These Calendars" ) );
  mActionManager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete Calendar", "Delete Calendars" ) );
  mActionManager->action( StandardActionManager::MoveCollectionToDialog )->setText( i18n( "Move Calendar To" ) );
  mActionManager->action( StandardActionManager::CopyCollectionToDialog )->setText( i18n( "Copy Calendar To" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize All Accounts" ) );

  const QStringList pages = QStringList() << QLatin1String( "CalendarSupport::CollectionGeneralPage" )
                                          << QLatin1String( "Akonadi::CachePolicyPage" );

  mActionManager->setCollectionPropertiesPageNames( pages );
}

void MainView::updateActionTexts()
{
  const Akonadi::Item::List items = mActionManager->selectedItems();
  if ( items.count() < 1 )
    return;

  const int itemCount = items.count();
  const Akonadi::Item item = items.first();
  const QString mimeType = item.mimeType();
  if ( mimeType == KCalCore::Event::eventMimeType() ) {
    actionCollection()->action( "akonadi_item_copy" )->setText( ki18np( "Copy Event", "Copy %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_copy_to_dialog" )->setText( i18n( "Copy Event To" ) );
    actionCollection()->action( "akonadi_item_delete" )->setText( ki18np( "Delete Event", "Delete %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_move_to_dialog" )->setText( i18n( "Move Event To" ) );
    actionCollection()->action( "akonadi_incidence_edit" )->setText( i18n( "Edit Event" ) );
  } else if ( mimeType == KCalCore::Todo::todoMimeType() ) {
    actionCollection()->action( "akonadi_item_copy" )->setText( ki18np( "Copy Task", "Copy %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_copy_to_dialog" )->setText( i18n( "Copy Task To" ) );
    actionCollection()->action( "akonadi_item_delete" )->setText( ki18np( "Delete Task", "Delete %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_move_to_dialog" )->setText( i18n( "Move Task To" ) );
    actionCollection()->action( "akonadi_incidence_edit" )->setText( i18n( "Edit Task" ) );
  } else if ( mimeType == KCalCore::Journal::journalMimeType() ) {
    actionCollection()->action( "akonadi_item_copy" )->setText( ki18np( "Copy Journal", "Copy %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_copy_to_dialog" )->setText( i18n( "Copy Journal To" ) );
    actionCollection()->action( "akonadi_item_delete" )->setText( ki18np( "Delete Journal", "Delete %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( "akonadi_item_move_to_dialog" )->setText( i18n( "Move Journal To" ) );
    actionCollection()->action( "akonadi_incidence_edit" )->setText( i18n( "Edit Journal" ) );
  }
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
  return new EventsFilterProxyModel();
}

ImportHandlerBase* MainView::importHandler() const
{
  return new EventsImportHandler();
}

ExportHandlerBase* MainView::exportHandler() const
{
  return new EventsExportHandler();
}

GuiStateManager* MainView::createGuiStateManager() const
{
  return new EventsGuiStateManager();
}

bool MainView::useFilterLineEditInCurrentState() const
{
  return (guiStateManager()->currentState() == EventsGuiStateManager::ViewEventListState);
}

void MainView::uploadFreeBusy()
{
  CalendarSupport::FreeBusyManager::self()->publishFreeBusy( this );
}

void MainView::mailFreeBusy()
{
  CalendarSupport::FreeBusyManager::self()->mailFreeBusy( 30, this );
}

void MainView::sendAsICalendar()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchForSendICalDone( KJob* ) ) );
}

void MainView::fetchForSendICalDone( KJob *job )
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

  CalendarSupport::sendAsICalendar( item, m_identityManager, this );
}

void MainView::publishItemInformation()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchForPublishItemDone( KJob* ) ) );
}

void MainView::fetchForPublishItemDone( KJob *job )
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

  CalendarSupport::publishItemInformation( item, m_calendar, this );
}

void MainView::sendInvitation()
{
  scheduleiTIPMethod( KCalCore::iTIPRequest );
}

void MainView::sendStatusUpdate()
{
  scheduleiTIPMethod( KCalCore::iTIPReply );
}

void MainView::sendCancellation()
{
  scheduleiTIPMethod( KCalCore::iTIPCancel );
}

void MainView::requestUpdate()
{
  scheduleiTIPMethod( KCalCore::iTIPRefresh );
}

void MainView::requestChange()
{
  scheduleiTIPMethod( KCalCore::iTIPCounter );
}

void MainView::scheduleiTIPMethod( KCalCore::iTIPMethod method )
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  job->setProperty( "iTIPmethod", QVariant::fromValue<KCalCore::iTIPMethod>( method ) );
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchForiTIPMethodDone( KJob* ) ) );
}

void MainView::fetchForiTIPMethodDone( KJob *job )
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

  const KCalCore::iTIPMethod method = job->property( "iTIPmethod" ).value<KCalCore::iTIPMethod>();
  CalendarSupport::scheduleiTIPMethods( method, item, m_calendar, this );
}

void MainView::saveAllAttachments()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL( result( KJob* ) ), this, SLOT( fetchForSaveAllAttachmentsDone( KJob* ) ) );
}

void MainView::fetchForSaveAllAttachmentsDone( KJob *job )
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
  CalendarSupport::ArchiveDialog archiveDialog( m_calendar, m_changer, this );
  archiveDialog.exec();
}

void MainView::changeCalendarColor()
{
  EventViews::AgendaViewItem *agendaItem = rootObject()->findChild<EventViews::AgendaViewItem*>();

  if ( !agendaItem )
    return; //something is fishy

  const QItemSelectionModel *collectionSelectionModel = regularSelectionModel();
  if ( collectionSelectionModel->selection().indexes().isEmpty() )
     return;

  const QModelIndexList selectedIndexes = collectionSelectionModel->selection().indexes();
  const Collection collection = selectedIndexes.first().data( CollectionModel::CollectionRole ).value<Collection>();
  QString id = QString::number( collection.id() );
  QColor calendarColor = agendaItem->preferences()->resourceColor( id );
  QColor myColor;

//FIXME: WINCE we have disabled QTableWidget for now, so the kcolordialog does not work yet.
#ifndef _WIN32_WCE
  const int result = KColorDialog::getColor( myColor, calendarColor );
  if ( result == KColorDialog::Accepted && myColor != calendarColor ) {
    agendaItem->preferences()->setResourceColor( id, myColor );
    agendaItem->updateConfig();

    EventViews::MonthViewItem *monthItem = rootObject()->findChild<EventViews::MonthViewItem*>();

    if ( monthItem ) {
      monthItem->preferences()->setResourceColor( id, myColor );
      monthItem->updateConfig();
    }
  }
#endif
}




#include "mainview.moc"
