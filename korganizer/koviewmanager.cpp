/*
    This file is part of KOrganizer.

    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <tqwidgetstack.h>
#include <tqtabwidget.h>

#include <libkcal/calendarresources.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kglobal.h>

#include "actionmanager.h"
#include "calendarview.h"
#include "datenavigator.h"
#include "kotodoview.h"
#include "koagendaview.h"
#include "komonthview.h"
#include "kolistview.h"
#include "kowhatsnextview.h"
#include "kojournalview.h"
#include "kotimelineview.h"
#include "koprefs.h"
#include "koglobals.h"
#include "navigatorbar.h"
#include "multiagendaview.h"
#include <korganizer/mainwindow.h>

#include "koviewmanager.h"
#include "koviewmanager.moc"

KOViewManager::KOViewManager( CalendarView *mainView ) :
  TQObject(), mMainView( mainView )
{
  mCurrentView = 0;

  mLastEventView = 0;

  mWhatsNextView = 0;
  mTodoView = 0;
  mAgendaView = 0;
  mAgendaSideBySideView = 0;
  mMonthView = 0;
  mListView = 0;
  mJournalView = 0;
  mTimelineView = 0;
  mAgendaViewTabs = 0;
  mAgendaViewTabIndex = 0;
  mAgendaMode = AGENDA_NONE;
}

KOViewManager::~KOViewManager()
{
}


KOrg::BaseView *KOViewManager::currentView()
{
  return mCurrentView;
}

void KOViewManager::readSettings(KConfig *config)
{
  config->setGroup("General");
  TQString view = config->readEntry("Current View");

  if (view == "WhatsNext") showWhatsNextView();
  else if (view == "Month") showMonthView();
  else if (view == "List") showListView();
  else if (view == "Journal") showJournalView();
  else if (view == "Todo") showTodoView();
  else if (view == "Timeline") showTimelineView();
  else {
    mAgendaMode = AgendaMode( config->readNumEntry( "Agenda Mode", AGENDA_OTHER ) );

    switch ( mAgendaMode ) {
      case AGENDA_WORK_WEEK:
        showWorkWeekView();
        break;
      case AGENDA_WEEK:
        showWeekView();
        break;
      case AGENDA_NEXTX:
        showNextXView();
        break;
      case AGENDA_DAY:
        showDayView();
        break;
      case AGENDA_NONE:
        // Someone has been playing with the config file.
      default:
        mAgendaMode = AGENDA_OTHER;
        showAgendaView();
    }
  }
}

void KOViewManager::writeSettings(KConfig *config)
{
  config->setGroup("General");

  TQString view;
  if (mCurrentView == mWhatsNextView) view = "WhatsNext";
  else if (mCurrentView == mMonthView) view = "Month";
  else if (mCurrentView == mListView) view = "List";
  else if (mCurrentView == mJournalView) view = "Journal";
  else if (mCurrentView == mTodoView) view = "Todo";
  else if (mCurrentView == mTimelineView) view = "Timeline";
  else {
    view = "Agenda";
    config->writeEntry( "Agenda Mode", mAgendaMode );
  }

  config->writeEntry("Current View",view);

  if (mAgendaView) {
    mAgendaView->writeSettings(config);
  }
  if (mListView) {
    mListView->writeSettings(config);
  }
  if (mTodoView) {
    mTodoView->saveLayout(config,"Todo View");
  }
}

void KOViewManager::showView(KOrg::BaseView *view)
{
  if( view == mCurrentView ) return;

  mCurrentView = view;

  if ( mCurrentView && mCurrentView->isEventView() ) {
    mLastEventView = mCurrentView;
  }

  if ( mAgendaView ) mAgendaView->deleteSelectedDateTime();

  raiseCurrentView();

  mMainView->processIncidenceSelection( 0, TQDate() );

  mMainView->updateView();

  mMainView->adaptNavigationUnits();
}

void KOViewManager::goMenu( bool enable )
{
  KOrg::MainWindow *w = ActionManager::findInstance( KURL() );
  if ( w ) {
    KActionCollection *ac = w->getActionCollection();
    if ( ac ) {
      KAction *action;
      action = ac->action( "go_today" );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( "go_previous" );
      if ( action ) {
        action->setEnabled( enable );
      }
      action = ac->action( "go_next" );
      if ( action ) {
        action->setEnabled( enable );
      }
    }
  }
}

void KOViewManager::raiseCurrentView()
{
  if ((mMonthView && KOPrefs::instance()->mFullViewMonth && mCurrentView == mMonthView) ||
      (mTodoView && KOPrefs::instance()->mFullViewTodo && mCurrentView == mTodoView)) {
    mMainView->showLeftFrame( false );
    if ( mCurrentView == mTodoView ) {
      mMainView->navigatorBar()->hide();
    } else {
      mMainView->navigatorBar()->show();
    }
  } else {
    mMainView->showLeftFrame( true );
    mMainView->navigatorBar()->hide();
  }
  mMainView->viewStack()->raiseWidget( widgetForView( mCurrentView  ) );
}

void KOViewManager::updateView()
{
  if ( mCurrentView ) mCurrentView->updateView();
}

void KOViewManager::updateView(const TQDate &start, const TQDate &end)
{
//  kdDebug(5850) << "KOViewManager::updateView()" << endl;

  if (mCurrentView) mCurrentView->showDates(start, end);

  if (mTodoView) mTodoView->updateView();
}

void KOViewManager::connectView(KOrg::BaseView *view)
{
  if (!view) return;

  // selecting an incidence
  connect( view, TQT_SIGNAL( incidenceSelected( Incidence *,const TQDate & ) ),
           mMainView, TQT_SLOT( processMainViewSelection( Incidence *,const TQDate & ) ) );

  // showing/editing/deleting an incidence. The calendar view takes care of the action.
  connect( view, TQT_SIGNAL(showIncidenceSignal(Incidence *,const TQDate &)),
           mMainView, TQT_SLOT(showIncidence(Incidence *,const TQDate &)) );
  connect( view, TQT_SIGNAL(editIncidenceSignal(Incidence *,const TQDate &)),
           mMainView, TQT_SLOT(editIncidence(Incidence *,const TQDate &)) );
  connect( view, TQT_SIGNAL(deleteIncidenceSignal(Incidence *)),
           mMainView, TQT_SLOT(deleteIncidence(Incidence *)) );
  connect( view, TQT_SIGNAL(copyIncidenceSignal(Incidence *)),
           mMainView, TQT_SLOT(copyIncidence(Incidence *)) );
  connect( view, TQT_SIGNAL(cutIncidenceSignal(Incidence *)),
           mMainView, TQT_SLOT(cutIncidence(Incidence *)) );
  connect( view, TQT_SIGNAL(pasteIncidenceSignal()),
           mMainView, TQT_SLOT(pasteIncidence()));
  connect( view, TQT_SIGNAL(toggleAlarmSignal(Incidence *)),
           mMainView, TQT_SLOT(toggleAlarm(Incidence *)) );
  connect( view,TQT_SIGNAL(dissociateOccurrenceSignal(Incidence *,const TQDate &)),
           mMainView, TQT_SLOT(dissociateOccurrence(Incidence *,const TQDate &)) );
  connect( view,TQT_SIGNAL(dissociateFutureOccurrenceSignal(Incidence *,const TQDate &)),
           mMainView, TQT_SLOT(dissociateFutureOccurrence(Incidence *,const TQDate &)) );

  // signals to create new incidences
  connect( view, TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &)),
           mMainView, TQT_SLOT(newEvent(ResourceCalendar *,const TQString &)) );
  connect( view, TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDate &)),
           mMainView, TQT_SLOT(newEvent(ResourceCalendar *,const TQString &,const TQDate &)) );
  connect( view, TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &)),
           mMainView, TQT_SLOT(newEvent(ResourceCalendar *,const TQString &,const TQDateTime &)) );
  connect( view, TQT_SIGNAL(newEventSignal(ResourceCalendar *,const TQString &,const TQDateTime &,const TQDateTime &)),
           mMainView, TQT_SLOT(newEvent(ResourceCalendar *,const TQString &,const TQDateTime &,const TQDateTime &)) );

  connect( view, TQT_SIGNAL(newTodoSignal(ResourceCalendar *,const TQString &,const TQDate &)),
           mMainView, TQT_SLOT(newTodo(ResourceCalendar *,const TQString &,const TQDate &)) );
  connect( view, TQT_SIGNAL(newSubTodoSignal(Todo *)),
           mMainView, TQT_SLOT(newSubTodo(Todo *)) );

  connect( view, TQT_SIGNAL(newJournalSignal(ResourceCalendar *,const TQString &,const TQDate &)),
           mMainView, TQT_SLOT(newJournal(ResourceCalendar *,const TQString &,const TQDate &)) );

  // reload settings
  connect(mMainView, TQT_SIGNAL(configChanged()), view, TQT_SLOT(updateConfig()));

  // Notifications about added, changed and deleted incidences
  connect( mMainView, TQT_SIGNAL( dayPassed( const TQDate & ) ),
           view, TQT_SLOT( dayPassed( const TQDate & ) ) );
  connect( view, TQT_SIGNAL( startMultiModify( const TQString & ) ),
           mMainView, TQT_SLOT( startMultiModify( const TQString & ) ) );
  connect( view, TQT_SIGNAL( endMultiModify() ),
           mMainView, TQT_SLOT( endMultiModify() ) );

  connect( mMainView, TQT_SIGNAL( newIncidenceChanger( IncidenceChangerBase* ) ),
           view, TQT_SLOT( setIncidenceChanger( IncidenceChangerBase * ) ) );
  view->setIncidenceChanger( mMainView->incidenceChanger() );
}

void KOViewManager::connectTodoView( KOTodoView* todoView )
{
  if (!todoView) return;

  // SIGNALS/SLOTS FOR TODO VIEW
  connect( todoView, TQT_SIGNAL( purgeCompletedSignal() ),
           mMainView, TQT_SLOT( purgeCompleted() ) );
  connect( todoView, TQT_SIGNAL( unSubTodoSignal() ),
           mMainView, TQT_SLOT( todo_unsub() ) );
  connect( todoView, TQT_SIGNAL( unAllSubTodoSignal() ),
           mMainView, TQT_SLOT( makeSubTodosIndependent() ) );
}

void KOViewManager::zoomInHorizontally()
{
  if( mAgendaView == mCurrentView ) mAgendaView->zoomInHorizontally();
}
void KOViewManager::zoomOutHorizontally()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomOutHorizontally();
}
void KOViewManager::zoomInVertically()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomInVertically();
}
void KOViewManager::zoomOutVertically()
{
  if( mAgendaView== mCurrentView ) mAgendaView->zoomOutVertically();
}

void KOViewManager::addView(KOrg::BaseView *view)
{
  connectView( view );
#if QT_VERSION >= 300
  mMainView->viewStack()->addWidget( view );
#else
  mMainView->viewStack()->addWidget( view, 1 );
#endif
}

void KOViewManager::showWhatsNextView()
{
  if (!mWhatsNextView) {
    mWhatsNextView = new KOWhatsNextView(mMainView->calendar(),mMainView->viewStack(),
                                         "KOViewManager::WhatsNextView");
    addView(mWhatsNextView);
  }
  goMenu( true );
  showView(mWhatsNextView);
}

void KOViewManager::showListView()
{
  if (!mListView) {
    mListView = new KOListView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::ListView");
    addView(mListView);
  }
  goMenu( true );
  showView(mListView);
}

void KOViewManager::showAgendaView()
{
  // If the user opens a local file, through menu->open ( for example ), then
  // it doesn't make sense to use multiagenda.
  CalendarResources *calres = dynamic_cast<CalendarResources*>( mMainView->calendar() );
  bool isLocalFile = !calres;

  int mode = KOPrefs::instance()->agendaViewCalendarDisplay();

  const bool showBoth = ( mode == KOPrefs::AllCalendarViews && !isLocalFile );

  const bool showMerged = showBoth || mode == KOPrefs::CalendarsMerged || isLocalFile;

  const bool showSideBySide = !isLocalFile && ( showBoth || mode == KOPrefs::CalendarsSideBySide );

  TQWidget *parent = mMainView->viewStack();
  if ( !mAgendaViewTabs && showBoth ) {
    mAgendaViewTabs = new TQTabWidget( mMainView->viewStack() );
    connect( mAgendaViewTabs, TQT_SIGNAL( currentChanged( TQWidget* ) ),
             this, TQT_SLOT( currentAgendaViewTabChanged( TQWidget* ) ) );
    parent = mAgendaViewTabs;

    KConfig *config = KOGlobals::self()->config();
    config->setGroup( "Views" );
    mAgendaViewTabIndex = config->readNumEntry( "Agenda View Tab Index", 0 );
  }

  if ( !mAgendaView && showMerged ) {
    mAgendaView = new KOAgendaView( mMainView->calendar(),
                                    mMainView,
                                    parent,
                                    "KOViewManager::AgendaView" );

    addView(mAgendaView);

    connect(mAgendaView, TQT_SIGNAL( toggleExpand() ),
            mMainView, TQT_SLOT( toggleExpand() ) );
    connect(mMainView, TQT_SIGNAL( calendarViewExpanded( bool ) ),
            mAgendaView, TQT_SLOT( setExpandedButton( bool ) ) );

    connect( mAgendaView,TQT_SIGNAL( zoomViewHorizontally(const TQDate &, int )),
             mMainView->dateNavigator(),TQT_SLOT( selectDates( const TQDate &, int ) ) );
    mAgendaView->readSettings();
  }

  if ( !mAgendaSideBySideView && showSideBySide ) {
    mAgendaSideBySideView =
      new MultiAgendaView( mMainView->calendar(), mMainView, parent,
                        "KOViewManager::AgendaSideBySideView" );

    addView(mAgendaSideBySideView);

/*    connect(mAgendaSideBySideView, TQT_SIGNAL( toggleExpand() ),
            mMainView, TQT_SLOT( toggleExpand() ) );
    connect(mMainView, TQT_SIGNAL( calendarViewExpanded( bool ) ),
            mAgendaSideBySideView, TQT_SLOT( setExpandedButton( bool ) ) );

    connect( mAgendaSideBySideView,TQT_SIGNAL( zoomViewHorizontally(const TQDate &, int )),
             mMainView->dateNavigator(),TQT_SLOT( selectDates( const TQDate &, int ) ) );*/
  }

  if ( showBoth && mAgendaViewTabs ) {
    if ( mAgendaView && mAgendaViewTabs->indexOf( mAgendaView ) < 0 )
      mAgendaViewTabs->addTab( mAgendaView, i18n("Merged calendar") );
    if ( mAgendaSideBySideView && mAgendaViewTabs->indexOf( mAgendaSideBySideView ) < 0 )
      mAgendaViewTabs->addTab( mAgendaSideBySideView, i18n("Calendars Side by Side") );
    mAgendaViewTabs->setCurrentPage( mAgendaViewTabIndex );
  } else {
    if ( mAgendaView && mMainView->viewStack()->id( mAgendaView ) < 0 )
      mMainView->viewStack()->addWidget( mAgendaView );
    if ( mAgendaSideBySideView && mMainView->viewStack()->id( mAgendaSideBySideView ) < 0 )
      mMainView->viewStack()->addWidget( mAgendaSideBySideView );
  }

  goMenu( true );
  if ( mAgendaViewTabs && showBoth )
    showView( static_cast<KOrg::BaseView*>( mAgendaViewTabs->currentPage() ) );
  else if ( mAgendaView && showMerged )
    showView( mAgendaView );
  else if ( mAgendaSideBySideView && showSideBySide )
    showView( mAgendaSideBySideView );
}

void KOViewManager::showDayView()
{
  mAgendaMode = AGENDA_DAY;
  showAgendaView();
  mMainView->dateNavigator()->selectDates( 1 );
}

void KOViewManager::showWorkWeekView()
{
  mAgendaMode = AGENDA_WORK_WEEK;
  showAgendaView();
  mMainView->dateNavigator()->selectWorkWeek();
}

void KOViewManager::showWeekView()
{
  mAgendaMode = AGENDA_WEEK;
  showAgendaView();
  mMainView->dateNavigator()->selectWeek();
}

void KOViewManager::showNextXView()
{
  mAgendaMode = AGENDA_NEXTX;
  showAgendaView();
  mMainView->dateNavigator()->selectDates( TQDate::currentDate(),
                                           KOPrefs::instance()->mNextXDays );
}

void KOViewManager::showMonthView()
{
  if (!mMonthView) {
    mMonthView = new KOMonthView(mMainView->calendar(), mMainView->viewStack(), "KOViewManager::MonthView");
    addView(mMonthView);
  }

  goMenu( true );
  showView(mMonthView);
}

void KOViewManager::showTodoView()
{
  if ( !mTodoView ) {
    mTodoView = new KOTodoView( mMainView->calendar(), mMainView->viewStack(),
                                "KOViewManager::TodoView" );
    mTodoView->setCalendar( mMainView->calendar() );
    addView( mTodoView );
    connectTodoView( mTodoView );

    KConfig *config = KOGlobals::self()->config();
    mTodoView->restoreLayout( config, "Todo View" );
  }

  goMenu( false );
  showView( mTodoView );
}

void KOViewManager::showJournalView()
{
  if (!mJournalView) {
    mJournalView = new KOJournalView(mMainView->calendar(),mMainView->viewStack(),
                                     "KOViewManager::JournalView");
    addView(mJournalView);
  }

  goMenu( true );
  showView(mJournalView);
}


void KOViewManager::showTimelineView()
{
  if (!mTimelineView) {
    mTimelineView = new KOTimelineView(mMainView->calendar(),mMainView->viewStack(),
                                     "KOViewManager::TimelineView");
    addView(mTimelineView);
  }
  goMenu( true );
  showView(mTimelineView);
}

void KOViewManager::showEventView()
{
  if ( mLastEventView ) {
    goMenu( true );
    showView( mLastEventView );
  } else {
    showWeekView();
  }
}

Incidence *KOViewManager::currentSelection()
{
  if ( !mCurrentView ) return 0;
  Incidence::List incidenceList = mCurrentView->selectedIncidences();
  if ( incidenceList.isEmpty() ) return 0;

  return incidenceList.first();
}

TQDate KOViewManager::currentSelectionDate()
{
  TQDate qd;
  if (mCurrentView) {
    DateList qvl = mCurrentView->selectedIncidenceDates();
    if (!qvl.isEmpty()) qd = qvl.first();
  }
  return qd;
}

void KOViewManager::setDocumentId( const TQString &id )
{
  if (mTodoView) mTodoView->setDocumentId( id );
}


TQWidget* KOViewManager::widgetForView( KOrg::BaseView* view ) const
{
  const bool showBoth = KOPrefs::instance()->agendaViewCalendarDisplay() == KOPrefs::AllCalendarViews;
  if ( (view == mAgendaView || view == mAgendaSideBySideView) && mAgendaViewTabs && showBoth ) {
    return mAgendaViewTabs;
  }
  return view;
}


void KOViewManager::currentAgendaViewTabChanged( TQWidget* widget )
{
  KConfig *config = KOGlobals::self()->config();
  config->setGroup( "Views" );
  config->writeEntry( "Agenda View Tab Index", mAgendaViewTabs->currentPageIndex() );

  goMenu( true );
  showView( static_cast<KOrg::BaseView*>( widget ) );
}

void KOViewManager::resourcesChanged()
{
  if ( mAgendaView )
    mAgendaView->resourcesChanged();
  if ( mAgendaSideBySideView )
    mAgendaSideBySideView->resourcesChanged();
}

void KOViewManager::updateMultiCalendarDisplay()
{
  if ( agendaIsSelected() ) {
    showAgendaView();
  } else {
    updateView();
  }
}

bool KOViewManager::agendaIsSelected() const
{
  return mCurrentView == mAgendaView            ||
         mCurrentView == mAgendaSideBySideView  ||
        ( mAgendaViewTabs && mCurrentView == mAgendaViewTabs->currentPage() );
}
