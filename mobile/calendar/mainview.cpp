/*
* This file is part of Akonadi
*
* Copyright (c) 2010 Volker Krause <vkrause@kde.org>
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

#include <kcalcore/event.h>
#include <kcalcore/todo.h>
#include <calendarsupport/calendar.h>
#include <calendarsupport/kcalprefs.h>

#include <akonadi/agentactionmanager.h>
#include <akonadi/entitytreemodel.h>
#include <Akonadi/ItemFetchScope>
#include <akonadi/standardactionmanager.h>
#include <incidenceeditors/incidenceeditor-ng/incidencedefaults.h>

#include <ksystemtimezone.h>

#include <qdeclarativeengine.h>
#include <qdeclarativecontext.h>

#include "agendaviewitem.h"
#include "qmldateedit.h"
#include "calendar/incidenceview.h"
#include "calendar/kcalitembrowseritem.h"

#include <KAction>
#include <KActionCollection>

#include <QGraphicsItem>
#include <QTimer>
#include <QDBusConnection>

using namespace Akonadi;
using CalendarSupport::KCalPrefs;

QML_DECLARE_TYPE( CalendarSupport::KCal::KCalItemBrowserItem )
QML_DECLARE_TYPE( EventViews::AgendaView )
QML_DECLARE_TYPE( Qt::QmlDateEdit )

MainView::MainView( QWidget* parent ) : KDeclarativeMainView( "korganizer-mobile", 0 /* TODO */, parent )
{
  m_calendar = 0;
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
  qmlRegisterType<Qt::QmlDateEdit>( "org.qt", 4, 7, "QmlDateEdit" );

  m_calendar = new CalendarSupport::Calendar( entityTreeModel(), regularSelectedItems(), KSystemTimeZones::local() );
  engine()->rootContext()->setContextProperty( "calendarModel", QVariant::fromValue( static_cast<QObject*>( m_calendar ) ) );

  QDBusConnection::sessionBus().registerService("org.kde.korganizer"); //register also as the real korganizer, so kmail can communicate with it
  CalendarInterface* calendarIface = new CalendarInterface();
  new CalendarAdaptor(calendarIface);
  QDBusConnection::sessionBus().registerObject("/Calendar", calendarIface);

  KAction *action = new KAction( i18n( "New Appointment" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newEvent()) );
  actionCollection()->addAction( QLatin1String( "add_new_event" ), action );
  action = new KAction( i18n( "New Todo" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(newTodo()) );
  actionCollection()->addAction( QLatin1String( "add_new_task" ), action );

  //connect Qt signals to QML slots
  connect(calendarIface, SIGNAL(showDateSignal(QVariant)), rootObject(), SLOT(showDate(QVariant)));
  connect(calendarIface, SIGNAL(showEventViewSignal()), rootObject(), SLOT(showEventView()));
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

  IncidenceEditorsNG::IncidenceDefaults defaults;
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
  IncidenceView *editor = new IncidenceView;
  editor->load( item, date );
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

  manager->action( Akonadi::StandardActionManager::SynchronizeResource )->setText( i18n( "Synchronize events\nin account" ) );
  manager->action( Akonadi::StandardActionManager::ResourceProperties )->setText( i18n( "Edit account" ) );
  manager->action( Akonadi::StandardActionManager::CreateCollection )->setText( i18n( "Add subfolder" ) );
  manager->action( Akonadi::StandardActionManager::DeleteCollections )->setText( i18n( "Delete folder" ) );
  manager->action( Akonadi::StandardActionManager::SynchronizeCollections )->setText( i18n( "Synchronize events\nin folder" ) );
  manager->action( Akonadi::StandardActionManager::CollectionProperties )->setText( i18n( "Edit folder" ) );
  manager->action( Akonadi::StandardActionManager::MoveCollectionToMenu )->setText( i18n( "Move folder to" ) );
  manager->action( Akonadi::StandardActionManager::CopyCollectionToMenu )->setText( i18n( "Copy folder to" ) );
  manager->action( Akonadi::StandardActionManager::DeleteItems )->setText( i18n( "Delete event" ) );
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

#include "mainview.moc"
