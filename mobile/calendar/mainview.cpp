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
#include "calendar/clockhelper.h"
#include "calendar/groupwareuidelegate.h"
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
#include "settings.h"
#include "timelineviewitem.h"
#include "qmldateedit.h"

#include <AkonadiWidgets/agentactionmanager.h>
#include <Akonadi/Calendar/StandardCalendarActionManager>
#include <Akonadi/Calendar/IncidenceChanger>
#include <Akonadi/Calendar/ITIPHandler>
#include <Akonadi/Calendar/FreeBusyManager>
#include <akonadi/calendar/calendarsettings.h>
#include <AkonadiCore/collectionmodel.h>
#include <AkonadiWidgets/collectionpropertiesdialog.h>
#include <AkonadiCore/entitytreemodel.h>
#include <AkonadiCore/itemfetchjob.h>
#include <AkonadiCore/itemfetchscope.h>
#include <AkonadiWidgets/standardactionmanager.h>
#include <calendarsupport/archivedialog.h>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/collectiongeneralpage.h>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/identitymanager.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>
#include <calendarviews/eventview.h>
#include <calendarviews/agenda/agendaview.h>
#include <calendarviews/month/monthview.h>
#include <kaction.h>
#include <kactioncollection.h>
#include <KCalCore/Event>
#include <KCalCore/Todo>
#include <kcolorcombo.h>
#include <kcolordialog.h>
#include <kmessagebox.h>
#include <ksystemtimezone.h>
#include <incidenceeditor-ng/categoryeditdialog.h>
#include <incidenceeditor-ng/editorconfig.h>
#include <incidenceeditor-ng/incidencedefaults.h>
#include <libkdepimdbusinterfaces/reminderclient.h>

#include <QtCore/QTimer>
#include <QtDBus/QDBusConnection>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>
#include <QGraphicsItem>

#include <QDebug>

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
  : KDeclarativeMainView( QLatin1String("korganizer-mobile"), new EventListProxy, parent ),
    m_identityManager( 0 ),
    m_changer( 0 ),
    mActionManager( 0 )
{
  m_calendarPrefs = EventViews::PrefsPtr( new  EventViews::Prefs );
  m_calendarPrefs->readConfig();
  mITIPHandler = new Akonadi::ITIPHandler( this );

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

void MainView::doDelayedInit()
{
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
  qmlRegisterType<ClockHelper>( "ClockHelper", 4, 5, "ClockHelper" );
  qmlRegisterUncreatableType<EventsGuiStateManager>( "org.kde.akonadi.events", 4, 5, "EventsGuiStateManager", QLatin1String( "This type is only exported for its enums" ) );

  m_calendar = Akonadi::ETMCalendar::Ptr( new Akonadi::ETMCalendar() );
  m_calendar->setWeakPointer( m_calendar );
  engine()->rootContext()->setContextProperty( QLatin1String("calendarModel"), QVariant::fromValue( static_cast<QObject*>( m_calendar.data() ) ) );
  Akonadi::FreeBusyManager::self()->setCalendar( m_calendar );

  // TODO: set a groupware delegate to handle counter proposals

  m_changer = new Akonadi::IncidenceChanger( this );

  m_identityManager = new CalendarSupport::IdentityManager;

  // FIXME: My suspicion is that this is wrong. I.e. the collection selection is
  //        not correct resulting in no items showing up in the monthview.
  CalendarSupport::CollectionSelection *collectionselection;
  collectionselection = new CalendarSupport::CollectionSelection( regularSelectionModel(), this );
  EventViews::EventView::setGlobalCollectionSelection( collectionselection );

  QDBusConnection::sessionBus().registerService( QLatin1String("org.kde.korganizer") ); //register also as the real korganizer, so kmail can communicate with it

  QAction *action = new QAction( i18n( "Import Events" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(importItems()) );
  actionCollection()->addAction( QLatin1String( "import_events" ), action );

  action = new QAction( i18n( "Export Events From This Account" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_account_events" ), action );

  action = new QAction( i18n( "Export Displayed Events" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(exportItems()) );
  actionCollection()->addAction( QLatin1String( "export_selected_events" ), action );

  action = new QAction( i18n( "Archive Old Events" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(archiveOldEntries()) );
  actionCollection()->addAction( QLatin1String( "archive_old_entries" ), action );

  action = new QAction( i18n( "Publish Item Information" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(publishItemInformation()) );
  actionCollection()->addAction( QLatin1String( "publish_item_information" ), action );

  action = new QAction( i18n( "Send Invitations To Attendees" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(sendInvitation()) );
  actionCollection()->addAction( QLatin1String( "send_invitations_to_attendees" ), action );

  action = new QAction( i18n( "Send Status Update" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(sendStatusUpdate()) );
  actionCollection()->addAction( QLatin1String( "send_status_update" ), action );

  action = new QAction( i18n( "Send Cancellation To Attendees" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(sendCancellation()) );
  actionCollection()->addAction( QLatin1String( "send_cancellation_to_attendees" ), action );

  action = new QAction( i18n( "Request Update" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(requestUpdate()) );
  actionCollection()->addAction( QLatin1String( "request_update" ), action );

  action = new QAction( i18n( "Request Change" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(requestChange()) );
  actionCollection()->addAction( QLatin1String( "request_change" ), action );

  action = new QAction( i18n( "Send As ICalendar" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(sendAsICalendar()) );
  actionCollection()->addAction( QLatin1String( "send_as_icalendar" ), action );

  action = new QAction( i18n( "Mail Free Busy Information" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(mailFreeBusy()) );
  actionCollection()->addAction( QLatin1String( "mail_freebusy" ), action );

  action = new QAction( i18n( "Upload Free Busy Information" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(uploadFreeBusy()) );
  actionCollection()->addAction( QLatin1String( "upload_freebusy" ), action );

  action = new QAction( i18n( "Save All" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(saveAllAttachments()) );
  actionCollection()->addAction( QLatin1String( "save_all_attachments" ), action );

  action = new QAction( i18n( "Set Color Of Calendar" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(changeCalendarColor()) );
  actionCollection()->addAction( QLatin1String( "set_calendar_colour" ), action );

  action = new QAction( i18n( "Configure Categories" ), this );
  connect( action, SIGNAL(triggered(bool)), SLOT(configureCategories()) );
  actionCollection()->addAction( QLatin1String( "configure_categories" ), action );

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)),
           this, SLOT(qmlLoadingStateChanged(QDeclarativeView::Status)) );

  //register DBUS interface
  m_calendarIface = new CalendarInterface( this );
  new CalendarAdaptor( m_calendarIface );
  QDBusConnection::sessionBus().registerObject( QLatin1String("/Calendar"), m_calendarIface );

  KPIM::ReminderClient::startDaemon();
}

void MainView::qmlLoadingStateChanged( QDeclarativeView::Status status )
{
  if ( status != Ready ) // We wait until the QML is completely loaded
    return;

  connect( m_calendarIface, SIGNAL(showDateSignal(QVariant)),
           rootObject(), SLOT(showDate(QVariant)) );
  connect( m_calendarIface, SIGNAL(openIncidenceEditorSignal(QString,QString,QStringList,QStringList,QStringList,bool,KCalCore::Incidence::IncidenceType)),
           this, SLOT(openIncidenceEditor(QString,QString,QStringList,QStringList,QStringList,bool,KCalCore::Incidence::IncidenceType)) );

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
  //m_calendar->setUnfilteredModel( itemModel() );
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
  newEventWithDate( QDate::currentDate() );
}

void MainView::newEventWithDate( const QDate &date )
{
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "Kontact Touch Calendar" ) );
  Item item;
  item.setMimeType( KCalCore::Event::eventMimeType() );
  KCalCore::Event::Ptr event( new KCalCore::Event );

  IncidenceEditorNG::IncidenceDefaults defaults;

  {
    KDateTime dateTime = KDateTime::currentLocalDateTime();
    dateTime.setDate( date );
    defaults.setStartDateTime( dateTime );
  }
  // Set the full emails manually here, to avoid that we get dependencies on
  // KCalPrefs all over the place.
  defaults.setFullEmails( CalendarSupport::KCalPrefs::instance()->fullEmails() );
  // NOTE: At some point this should be generalized. That is, we now use the
  //       freebusy url as a hack, but this assumes that the user has only one
  //       groupware account. Which doesn't have to be the case necessarily.
  //       This method should somehow depend on the calendar selected to which
  //       the incidence is added.
  if ( KCalPrefs::instance()->useGroupwareCommunication() )
    defaults.setGroupWareDomain( KUrl( Akonadi::CalendarSettings::self()->freeBusyRetrieveUrl() ).host() );

  defaults.setDefaults( event );

  item.setPayload<KCalCore::Event::Ptr>( event );
  editor->load( item );

  if ( regularSelectionModel()->hasSelection() ) {
    const QModelIndex index = regularSelectionModel()->selectedIndexes().first();
    const Akonadi::Collection collection = index.data( Akonadi::EntityTreeModel::CollectionRole ).value<Akonadi::Collection>();
    if ( collection.isValid() )
      editor->setDefaultCollection( collection );
  }

  editor->show();
}

void MainView::newTodo()
{
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "Kontact Touch Calendar" ) );
  Item item;
  item.setMimeType( KCalCore::Todo::todoMimeType() );
  KCalCore::Todo::Ptr todo( new KCalCore::Todo );

  // make it due one day from now
  todo->setDtStart( KDateTime::currentLocalDateTime() );
  todo->setDtDue( KDateTime::currentLocalDateTime().addDays( 1 ) );

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

void MainView::openIncidenceEditor( const QString &summary,
                                    const QString &description,
                                    const QStringList &attachmentUris,
                                    const QStringList &attendees,
                                    const QStringList &attachmentMimeTypes,
                                    bool attachmentsAreInline,
                                    KCalCore::Incidence::IncidenceType type )
{
  qDebug();

  IncidenceEditorNG::IncidenceDefaults defaults = IncidenceEditorNG::IncidenceDefaults::minimalIncidenceDefaults( attachmentsAreInline );
  // if attach or attendee list is empty, these methods don't do anything, so
  // it's safe to call them in every case
  defaults.setAttachments( attachmentUris, attachmentMimeTypes, QStringList(), attachmentsAreInline );
  defaults.setAttendees( attendees );

  KCalCore::Incidence::Ptr incidence;

  if ( type == KCalCore::Incidence::TypeTodo ) {
    incidence = KCalCore::Incidence::Ptr( new KCalCore::Todo );
  } else if ( type == KCalCore::Incidence::TypeEvent ) {
    incidence = KCalCore::Incidence::Ptr( new KCalCore::Event );
  } else {
    Q_ASSERT_X( false, "openIncidenceEditor", "Unexpected incidence type" );
    return;
  }

  defaults.setDefaults( incidence );
  incidence->setSummary( summary );
  incidence->setDescription( description );

  Akonadi::Item item;
  item.setPayload( incidence );
  item.setMimeType( incidence->mimeType() );
  IncidenceView *editor = new IncidenceView;
  editor->setWindowTitle( i18n( "Kontact Touch Calendar" ) );
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
  editor->setWindowTitle( i18n( "Kontact Touch Calendar" ) );
  editor->load( item, date );

  m_openItemEditors.insert( editor, item.id() );
  connect( editor, SIGNAL(destroyed(QObject*)), SLOT(finishEdit(QObject*)) );

  editor->show();
}

void MainView::deleteIncidence()
{
  const QModelIndexList indexes = itemActionModel()->selectedRows();
  if ( indexes.isEmpty() )
    return;

  const Akonadi::Item item = indexes.first().data( Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
  deleteIncidence( item );
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

  connect( mActionManager->action( Akonadi::StandardActionManager::CreateResource ), SIGNAL(triggered(bool)),
           this, SLOT(launchAccountWizard()) );
  connect( mActionManager->action( Akonadi::StandardActionManager::DeleteItems ), SIGNAL(triggered(bool)),
           this, SLOT(deleteIncidence()) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::CreateEvent ), SIGNAL(triggered(bool)),
           this, SLOT(newEvent()) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::CreateTodo ), SIGNAL(triggered(bool)),
           this, SLOT(newTodo()) );
  connect( mActionManager->action( Akonadi::StandardCalendarActionManager::EditIncidence ), SIGNAL(triggered(bool)),
           this, SLOT(editIncidence()) );
  connect( mActionManager, SIGNAL(actionStateUpdated()), SLOT(updateActionTexts()) );

  ActionHelper::adaptStandardActionTexts( mActionManager );

  mActionManager->action( Akonadi::StandardCalendarActionManager::CreateEvent )->setText( i18n( "New Event" ) );
  mActionManager->action( StandardActionManager::CollectionProperties )->setText( i18n( "Calendar Properties" ) );
  mActionManager->action( StandardActionManager::CreateCollection )->setText( i18n( "New Sub Calendar" ) );
  mActionManager->action( StandardActionManager::CreateCollection )->setProperty( "ContentMimeTypes", QStringList( KCalCore::Event::eventMimeType() ) );
  mActionManager->setActionText( StandardActionManager::SynchronizeCollections, ki18np( "Synchronize This Calendar", "Synchronize These Calendars" ) );
  mActionManager->setActionText( StandardActionManager::DeleteCollections, ki18np( "Delete Calendar", "Delete Calendars" ) );
  mActionManager->action( StandardActionManager::MoveCollectionToDialog )->setText( i18n( "Move Calendar To" ) );
  mActionManager->action( StandardActionManager::CopyCollectionToDialog )->setText( i18n( "Copy Calendar To" ) );

  actionCollection()->action( QLatin1String("synchronize_all_items") )->setText( i18n( "Synchronize All Accounts" ) );

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
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Event", "Copy %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Event To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Event", "Delete %1 Events" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Event To" ) );
    actionCollection()->action( QLatin1String("akonadi_incidence_edit") )->setText( i18n( "Edit Event" ) );
  } else if ( mimeType == KCalCore::Todo::todoMimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Task", "Copy %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action(QLatin1String( "akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Task To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Task", "Delete %1 Tasks" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog") )->setText( i18n( "Move Task To" ) );
    actionCollection()->action( QLatin1String("akonadi_incidence_edit") )->setText( i18n( "Edit Task" ) );
  } else if ( mimeType == KCalCore::Journal::journalMimeType() ) {
    actionCollection()->action( QLatin1String("akonadi_item_copy") )->setText( ki18np( "Copy Journal", "Copy %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_copy_to_dialog") )->setText( i18n( "Copy Journal To" ) );
    actionCollection()->action( QLatin1String("akonadi_item_delete") )->setText( ki18np( "Delete Journal", "Delete %1 Journals" ).subs( itemCount ).toString() );
    actionCollection()->action( QLatin1String("akonadi_item_move_to_dialog" ))->setText( i18n( "Move Journal To" ) );
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

void MainView::configureCategories()
{
  CalendarSupport::CategoryConfig config( IncidenceEditorNG::EditorConfig::instance()->config(), 0 );
  IncidenceEditorNG::CategoryEditDialog dialog( &config, 0 );
  if ( dialog.exec() )
    config.writeConfig();
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
  Akonadi::FreeBusyManager::self()->publishFreeBusy( this );
}

void MainView::mailFreeBusy()
{
  Akonadi::FreeBusyManager::self()->mailFreeBusy( 30, this );
}

void MainView::sendAsICalendar()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL(result(KJob*)), this, SLOT(fetchForSendICalDone(KJob*)) );
}

void MainView::fetchForSendICalDone( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "Error trying to fetch item";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Cannot fetch calendar item." ),
                        i18n( "Item Fetch Error" ) );
    return;
  }

  const Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();

  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( incidence )
    mITIPHandler->sendAsICalendar( incidence, this );
}

void MainView::publishItemInformation()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL(result(KJob*)), this, SLOT(fetchForPublishItemDone(KJob*)) );
}

void MainView::fetchForPublishItemDone( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "Error trying to fetch item";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Cannot fetch calendar item." ),
                        i18n( "Item Fetch Error" ) );
    return;
  }

  const Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();


  KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( incidence )
    mITIPHandler->publishInformation( incidence, this );
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
  connect( job, SIGNAL(result(KJob*)), this, SLOT(fetchForiTIPMethodDone(KJob*)) );
}

void MainView::fetchForiTIPMethodDone( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "Error trying to fetch item";
    //###: review error string
    KMessageBox::sorry( this,
                        i18n( "Cannot fetch calendar item." ),
                        i18n( "Item Fetch Error" ) );
    return;
  }

  const Akonadi::Item item = static_cast<Akonadi::ItemFetchJob*>( job )->items().first();

  const KCalCore::iTIPMethod method = job->property( "iTIPmethod" ).value<KCalCore::iTIPMethod>();
  const KCalCore::Incidence::Ptr incidence = CalendarSupport::incidence( item );
  if ( incidence )
    mITIPHandler->sendiTIPMessage( method, incidence, this );
}

void MainView::saveAllAttachments()
{
  const QModelIndexList list = itemSelectionModel()->selectedIndexes();
  if ( list.isEmpty() )
    return;

  const Akonadi::Item item( list.first().data( EntityTreeModel::ItemIdRole ).toInt() );

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  connect( job, SIGNAL(result(KJob*)), this, SLOT(fetchForSaveAllAttachmentsDone(KJob*)) );
}

void MainView::fetchForSaveAllAttachmentsDone( KJob *job )
{
  if ( job->error() ) {
    qDebug() << "Error trying to fetch item";
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

  const int result = KColorDialog::getColor( myColor, calendarColor );

  if ( result == KDialog::Accepted && myColor != calendarColor ) {
    agendaItem->preferences()->setResourceColor( id, myColor );
    agendaItem->updateConfig();

    EventViews::MonthViewItem *monthItem = rootObject()->findChild<EventViews::MonthViewItem*>();

    if ( monthItem ) {
      monthItem->preferences()->setResourceColor( id, myColor );
      monthItem->updateConfig();
    }
  }
}




