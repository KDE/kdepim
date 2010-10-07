/*
* This file is part of Akonadi
*
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
#include "calendarinterface.h"
#include "calendaradaptor.h"
#include "eventlistproxy.h"
#include "eventsfilterproxymodel.h"
#include "eventsexporthandler.h"
#include "eventsimporthandler.h"

#include <kcalcore/event.h>
#include <kcalcore/todo.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarmodel.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/freebusymanager.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/identitymanager.h>
#include <calendarviews/eventviews/eventview.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/entitytreemodel.h>
#include <akonadi/itemfetchscope.h>
#include <akonadi/standardactionmanager.h>
#include <akonadi/itemfetchjob.h>
#include <incidenceeditor-ng/incidencedefaults.h>
#include <libkdepimdbusinterfaces/reminderclient.h>

#include <ksystemtimezone.h>

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

#include "agendaviewitem.h"
#include "monthviewitem.h"
#include "qmldateedit.h"
#include "calendar/incidenceview.h"
#include "calendar/kcalitembrowseritem.h"

#include <KAction>
#include <KActionCollection>
#include <KMessageBox>

#include <QGraphicsItem>
#include <QTimer>
#include <QDBusConnection>

Q_DECLARE_METATYPE(KCalCore::iTIPMethod)

using namespace Akonadi;
using CalendarSupport::KCalPrefs;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )
QML_DECLARE_TYPE( EventViews::AgendaView )
QML_DECLARE_TYPE( Qt::QmlDateEdit )

MainView::MainView( QWidget* parent )
  : KDeclarativeMainView( "korganizer-mobile", new EventListProxy, parent )
{
  m_calendar = 0;
  m_identityManager = 0;
}

MainView::~MainView()
{
  m_calendar->deleteLater();
}

void MainView::delayedInit()
{
  KDeclarativeMainView::delayedInit();
  setWindowTitle( i18n( "KOrganizer" ) );

  addMimeType( KCalCore::Event::eventMimeType() );
  addMimeType( KCalCore::Todo::todoMimeType() );
  itemFetchScope().fetchFullPayload();

  qmlRegisterType<CalendarSupport::KCal::KCalItemBrowserItem>( "org.kde.kcal", 4, 5, "IncidenceView" );
  qmlRegisterType<EventViews::AgendaViewItem>( "org.kde.calendarviews", 4, 5, "AgendaView" );
  qmlRegisterType<EventViews::MonthViewItem>( "org.kde.calendarviews", 4, 5, "MonthView" );
  qmlRegisterType<Qt::QmlDateEdit>( "org.qt", 4, 7, "QmlDateEdit" );

  m_calendar = new CalendarSupport::Calendar( entityTreeModel(), regularSelectedItems(), KSystemTimeZones::local() );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( m_calendar ) ) );
  CalendarSupport::FreeBusyManager::self()->setCalendar( m_calendar );

  m_identityManager = new CalendarSupport::IdentityManager;

  // FIXME: My suspicion is that this is wrong. I.e. the collection selection is
  //        not correct resulting in no items showing up in the monthview.
  CalendarSupport::CollectionSelection *collectionselection;
  collectionselection = new CalendarSupport::CollectionSelection( regularSelectionModel(), this );
  EventViews::EventView::setGlobalCollectionSelection( collectionselection );
  
  QDBusConnection::sessionBus().registerService("org.kde.korganizer"); //register also as the real korganizer, so kmail can communicate with it

  KAction *action = new KAction( i18n( "New Appointment" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newEvent()) );
  actionCollection()->addAction( QLatin1String( "add_new_event" ), action );
  action = new KAction( i18n( "New Todo" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newTodo()) );
  actionCollection()->addAction( QLatin1String( "add_new_task" ), action );

  action = new KAction( i18n( "Import Events" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( importItems() ) );
  actionCollection()->addAction( QLatin1String( "import_events" ), action );

  action = new KAction( i18n( "Export Events" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( exportItems() ) );
  actionCollection()->addAction( QLatin1String( "export_events" ), action );

  connect(this, SIGNAL(statusChanged(QDeclarativeView::Status)),
          this, SLOT(connectQMLSlots(QDeclarativeView::Status)));

  action = new KAction( i18n( "Publish Item Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( publishItemInformation()) );
  actionCollection()->addAction( QLatin1String( "publish_item_information" ), action );

  action = new KAction( i18n( "Send Invitation to Attendees" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendInvitation()) );
  actionCollection()->addAction( QLatin1String( "send_invitations_to_attendees" ), action );

  action = new KAction( i18n( "Send Status Update" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendStatusUpdate()) );
  actionCollection()->addAction( QLatin1String( "send_status_update" ), action );

  action = new KAction( i18n( "Send Cancellation to Attendees" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendCancellation()) );
  actionCollection()->addAction( QLatin1String( "send_cancellation_to_attendees" ), action );

  action = new KAction( i18n( "Request Update" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( requestUpdate()) );
  actionCollection()->addAction( QLatin1String( "request_update" ), action );

  action = new KAction( i18n( "Request Change" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( requestChange()) );
  actionCollection()->addAction( QLatin1String( "request_change" ), action );

  action = new KAction( i18n( "Send as ICalendar" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( sendAsICalendar()) );
  actionCollection()->addAction( QLatin1String( "send_as_icalendar" ), action );

  action = new KAction( i18n( "Mail Free Busy Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( mailFreeBusy()) );
  actionCollection()->addAction( QLatin1String( "mail_freebusy" ), action );

  action = new KAction( i18n( "Upload Free Busy Information" ), this );
  connect( action, SIGNAL( triggered( bool ) ), SLOT( uploadFreeBusy()) );
  actionCollection()->addAction( QLatin1String( "upload_freebusy" ), action );


  //register DBUS interface
  m_calendarIface = new CalendarInterface( this );
  new CalendarAdaptor(m_calendarIface);
  QDBusConnection::sessionBus().registerObject("/Calendar", m_calendarIface);

  KPIM::ReminderClient::startDaemon();
}

void MainView::connectQMLSlots(QDeclarativeView::Status status)
{
  if ( status != Ready )
    return;

  connect(m_calendarIface, SIGNAL(showDateSignal(QVariant)), rootObject(), SLOT(showDate(QVariant)));
  connect(m_calendarIface, SIGNAL(showEventViewSignal()), rootObject(), SLOT(showEventView()));
}

void MainView::finishEdit( QObject *editor )
{
  m_openItemEditors.remove( editor );
}

void MainView::showRegularCalendar()
{
  m_calendar->setUnfilteredModel(regularSelectedItems());
}

void MainView::setCurrentEventItemId(qint64 id)
{
  QModelIndexList list = itemModel()->match(itemModel()->index(0, 0), EntityTreeModel::ItemIdRole, id, 1 );
  if (list.isEmpty())
    return;

  setListSelectedRow(list.first().row());
}

void MainView::newEvent()
{
  IncidenceView *editor = new IncidenceView;
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

void MainView::editIncidence( const Akonadi::Item &item, const QDate &date )
{
  if ( m_openItemEditors.values().contains( item.id() ) )
    return; // An editor for this item is already open.

  IncidenceView *editor = new IncidenceView;
  editor->load( item, date );

  m_openItemEditors.insert(  editor, item.id() );
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

  manager->setActionText( Akonadi::StandardActionManager::SynchronizeResources, ki18np( "Synchronize events\nin account", "Synchronize events\nin accounts" ) );
  manager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  manager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteCollections, ki18np( "Delete folder", "Delete folders" ) );
  manager->setActionText( Akonadi::StandardActionManager::SynchronizeCollections, ki18np( "Synchronize events\nin folder", "Synchronize events\nin folders" ) );
  manager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  manager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  manager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  manager->setActionText( Akonadi::StandardActionManager::DeleteItems, ki18np( "Delete event", "Delete events" ) );
  manager->action( Akonadi::StandardActionManager::MoveItemToMenu )->setText( i18n( "Move event\nto folder" ) );
  manager->action( Akonadi::StandardActionManager::CopyItemToMenu )->setText( i18n( "Copy event\nto folder" ) );

  actionCollection()->action( "synchronize_all_items" )->setText( i18n( "Synchronize\nall events" ) );
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
  QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if (list.isEmpty())
    return;

  Akonadi::Item item( list.first().data(EntityTreeModel::ItemIdRole).toInt() );
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect(job, SIGNAL( result( KJob* ) ), this, SLOT(fetchForSendICalDone(KJob*)));  
}

void MainView::fetchForSendICalDone(KJob* job)
{
  if ( job->error() ) {
      kDebug() << "Error trying to fetch item";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n("Cannot fetch calendar item."),
                          i18n("Item Fetch Error"));
      return;
  }

  Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();
  
  CalendarSupport::sendAsICalendar( item, m_identityManager, this );
}

void MainView::publishItemInformation()
{
  QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if (list.isEmpty())
    return;

  Akonadi::Item item( list.first().data(EntityTreeModel::ItemIdRole).toInt() );
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect(job, SIGNAL( result( KJob* ) ), this, SLOT(fetchForPublishItemDone(KJob*)));
}

void MainView::fetchForPublishItemDone(KJob* job)
{
  if ( job->error() ) {
      kDebug() << "Error trying to fetch item";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n("Cannot fetch calendar item."),
                          i18n("Item Fetch Error"));
      return;
  }

  Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();

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
  QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if (list.isEmpty())
    return;

  Akonadi::Item item( list.first().data(EntityTreeModel::ItemIdRole).toInt() );
  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  job->setProperty( "iTIPmethod", QVariant::fromValue<KCalCore::iTIPMethod>(method) );
  connect(job, SIGNAL( result( KJob* ) ), this, SLOT(fetchForiTIPMethodDone(KJob*)));
}

void MainView::fetchForiTIPMethodDone(KJob* job)
{
  if ( job->error() ) {
      kDebug() << "Error trying to fetch item";
      //###: review error string
      KMessageBox::sorry( this,
                          i18n("Cannot fetch calendar item."),
                          i18n("Item Fetch Error"));
      return;
  }

  Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();

  KCalCore::iTIPMethod method = job->property( "iTIPmethod" ).value<KCalCore::iTIPMethod>();
  CalendarSupport::scheduleiTIPMethods( method, item, m_calendar, this );
}


#include "mainview.moc"
