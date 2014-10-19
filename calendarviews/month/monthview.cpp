/*
  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Bertjan Broeksema, broeksema@kde.org

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

#include "monthview.h"
#include "monthgraphicsitems.h"
#include "monthitem.h"
#include "monthscene.h"
#include "prefs.h"

#include <calendarsupport/collectionselection.h>
#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <KCalCore/OccurrenceIterator>
#include <KCheckableProxyModel>
#include <QIcon>
#include <QDebug>

#include <QHBoxLayout>
#include <QTimer>
#include <QToolButton>
#include <QWheelEvent>
#include <QLocale>

using namespace EventViews;

namespace EventViews {

class MonthViewPrivate : public Akonadi::ETMCalendar::CalendarObserver
{
  MonthView *q;

  public: /// Methods
    explicit MonthViewPrivate( MonthView *qq );

    void addIncidence( const Akonadi::Item &incidence );
    void moveStartDate( int weeks, int months );
    // void setUpModels();
    void triggerDelayedReload( EventView::Change reason );

  public:  /// Members
    QTimer                           reloadTimer;
    MonthScene                      *scene;
    QDate                            selectedItemDate;
    Akonadi::Item::Id                selectedItemId;
    MonthGraphicsView               *view;
    QToolButton                     *fullView;

    // List of uids for QDate
    QMap<QDate, QStringList > mBusyDays;

  protected:
    /* reimplemented from KCalCore::Calendar::CalendarObserver */
    void calendarIncidenceAdded( const KCalCore::Incidence::Ptr &incidence );
    void calendarIncidenceChanged( const KCalCore::Incidence::Ptr &incidence );
    void calendarIncidenceDeleted( const KCalCore::Incidence::Ptr &incidence );
};

}

MonthViewPrivate::MonthViewPrivate( MonthView *qq )
  : q( qq ),
    scene( new MonthScene( qq ) ),
    selectedItemId( -1 ),
    view( new MonthGraphicsView( qq ) ),
    fullView( 0 )
{
  reloadTimer.setSingleShot( true );
  view->setScene( scene );
}

void MonthViewPrivate::addIncidence( const Akonadi::Item &incidence )
{
  Q_UNUSED( incidence );
  //TODO: add some more intelligence here...
  q->setChanges( q->changes() | EventView::IncidencesAdded );
  reloadTimer.start( 50 );
}

void MonthViewPrivate::moveStartDate( int weeks, int months )
{
  KDateTime start = q->startDateTime();
  KDateTime end = q->endDateTime();
  start = start.addDays( weeks * 7 );
  end = end.addDays( weeks * 7 );
  start = start.addMonths( months );
  end = end.addMonths( months );

#ifndef KDEPIM_MOBILE_UI
  KCalCore::DateList dateList;
  QDate d = start.date();
  while ( d <= end.date() ) {
    dateList.append( d );
    d = d.addDays( 1 );
  }

  /**
   * If we call q->setDateRange( start, end ); directly,
   * it will change the selected dates in month view,
   * but the application won't know about it.
   * The correct way is to emit datesSelected()
   * #250256
   * */
  emit q->datesSelected( dateList );
#else
  // korg-mobile doesn't use korg's date navigator.
  // Before creating a solution with no #ifndef, we must first extract the remaining views from
  // korg, and review the API.
  q->setDateRange( start, end );
#endif
}

void MonthViewPrivate::triggerDelayedReload( EventView::Change reason )
{
  q->setChanges( q->changes() | reason );
  if ( !reloadTimer.isActive() ) {
    reloadTimer.start( 50 );
  }
}

void MonthViewPrivate::calendarIncidenceAdded( const KCalCore::Incidence::Ptr & )
{
  triggerDelayedReload( MonthView::IncidencesAdded );
}

void MonthViewPrivate::calendarIncidenceChanged( const KCalCore::Incidence::Ptr & )
{
  triggerDelayedReload( MonthView::IncidencesEdited );
}

void MonthViewPrivate::calendarIncidenceDeleted( const KCalCore::Incidence::Ptr &incidence )
{
  Q_ASSERT( !incidence->uid().isEmpty() );
  scene->removeIncidence( incidence->uid() );
}

/// MonthView

MonthView::MonthView( NavButtonsVisibility visibility, QWidget *parent )
  : EventView( parent ), d( new MonthViewPrivate( this ) )
{
  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->addWidget( d->view );
  topLayout->setMargin( 0 );

  if ( visibility == Visible ) {
    QVBoxLayout *rightLayout = new QVBoxLayout( );
    rightLayout->setSpacing( 0 );
    rightLayout->setMargin( 0 );

    // push buttons to the bottom
    rightLayout->addStretch( 1 );

    d->fullView = new QToolButton( this );
    d->fullView->setIcon( QIcon::fromTheme( QLatin1String("view-fullscreen") ) );
    d->fullView->setAutoRaise( true );
    d->fullView->setCheckable( true );
    d->fullView->setChecked( preferences()->fullViewMonth() );
    d->fullView->isChecked() ?
      d->fullView->setToolTip( i18nc( "@info:tooltip",
                                      "Display calendar in a normal size" ) ) :
      d->fullView->setToolTip( i18nc( "@info:tooltip",
                                      "Display calendar in a full window" ) );
    d->fullView->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Click this button and the month view will be enlarged to fill the "
             "maximum available window space / or shrunk back to its normal size." ) );
    connect( d->fullView, SIGNAL(clicked()),
             this, SLOT(changeFullView()) );

    QToolButton *minusMonth = new QToolButton( this );
    minusMonth->setIcon( QIcon::fromTheme( QLatin1String("arrow-up-double") ) );
    minusMonth->setAutoRaise( true );
    minusMonth->setToolTip( i18nc( "@info:tooltip", "Go back one month" ) );
    minusMonth->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Click this button and the view will be scrolled back in time by 1 month." ) );
    connect( minusMonth, SIGNAL(clicked()),
            this, SLOT(moveBackMonth()) );

    QToolButton *minusWeek = new QToolButton( this );
    minusWeek->setIcon( QIcon::fromTheme( QLatin1String("arrow-up") ) );
    minusWeek->setAutoRaise( true );
    minusWeek->setToolTip( i18nc( "@info:tooltip", "Go back one week" ) );
    minusWeek->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Click this button and the view will be scrolled back in time by 1 week." ) );
    connect( minusWeek, SIGNAL(clicked()),
            this, SLOT(moveBackWeek()) );

    QToolButton *plusWeek = new QToolButton( this );
    plusWeek->setIcon( QIcon::fromTheme( QLatin1String("arrow-down") ) );
    plusWeek->setAutoRaise( true );
    plusWeek->setToolTip( i18nc( "@info:tooltip", "Go forward one week" ) );
    plusWeek->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Click this button and the view will be scrolled forward in time by 1 week." ) );
    connect( plusWeek, SIGNAL(clicked()),
            this, SLOT(moveFwdWeek()) );

    QToolButton *plusMonth = new QToolButton( this );
    plusMonth->setIcon( QIcon::fromTheme( QLatin1String("arrow-down-double") ) );
    plusMonth->setAutoRaise( true );
    plusMonth->setToolTip( i18nc( "@info:tooltip", "Go forward one month" ) );
    plusMonth->setWhatsThis(
      i18nc( "@info:whatsthis",
             "Click this button and the view will be scrolled forward in time by 1 month." ) );
    connect( plusMonth, SIGNAL(clicked()),
            this, SLOT(moveFwdMonth()) );

    rightLayout->addWidget( d->fullView );
    rightLayout->addWidget( minusMonth );
    rightLayout->addWidget( minusWeek );
    rightLayout->addWidget( plusWeek );
    rightLayout->addWidget( plusMonth );

    topLayout->addLayout( rightLayout );
  } else {
    d->view->setFrameStyle( QFrame::NoFrame );
  }

  connect( d->scene, SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)),
           SIGNAL(showIncidencePopupSignal(Akonadi::Item,QDate)) );

  connect( d->scene, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->scene, SIGNAL(newEventSignal()),
           SIGNAL(newEventSignal()) );

  connect( d->scene, SIGNAL(showNewEventPopupSignal()),
           SIGNAL(showNewEventPopupSignal()) );

  connect(&d->reloadTimer, &QTimer::timeout, this, &MonthView::reloadIncidences);
  updateConfig();

  // d->setUpModels();
  d->reloadTimer.start( 50 );
}

MonthView::~MonthView()
{
  if ( calendar() ) {
    calendar()->unregisterObserver( d );
  }

  delete d;
}

void MonthView::updateConfig()
{
  d->scene->update();
  setChanges( changes() | ConfigChanged );
  d->reloadTimer.start( 50 );
}

int MonthView::currentDateCount() const
{
  return actualStartDateTime().date().daysTo( actualEndDateTime().date() );
}

KCalCore::DateList MonthView::selectedIncidenceDates() const
{
  KCalCore::DateList list;
  if ( d->scene->selectedItem() ) {
    IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( d->scene->selectedItem() );
    if ( tmp ) {
      QDate selectedItemDate = tmp->realStartDate();
      if ( selectedItemDate.isValid() ) {
        list << selectedItemDate;
      }
    }
  } else if ( d->scene->selectedCell() ) {
    list << d->scene->selectedCell()->date();
  }

  return list;
}

QDateTime MonthView::selectionStart() const
{
  if ( d->scene->selectedCell() ) {
    return QDateTime( d->scene->selectedCell()->date() );
  } else {
    return QDateTime();
  }
}

QDateTime MonthView::selectionEnd() const
{
  // Only one cell can be selected (for now)
  return selectionStart();
}

void MonthView::setDateRange( const KDateTime &start, const KDateTime &end,
                              const QDate &preferredMonth )
{
  EventView::setDateRange( start, end, preferredMonth );
  setChanges( changes() | DatesChanged );
  d->reloadTimer.start( 50 );
}

bool MonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const
{
  if ( d->scene->selectedCell() ) {
    startDt.setDate( d->scene->selectedCell()->date() );
    endDt.setDate( d->scene->selectedCell()->date() );
    allDay = true;
    return true;
  }

  return false;
}

void MonthView::showIncidences( const Akonadi::Item::List &incidenceList, const QDate &date )
{
  Q_UNUSED( incidenceList );
  Q_UNUSED( date );
}

void MonthView::changeIncidenceDisplay( const Akonadi::Item &incidence, int action )
{
  Q_UNUSED( incidence );
  Q_UNUSED( action );

  //TODO: add some more intelligence here...

  // don't call reloadIncidences() directly. It would delete all
  // MonthItems, but this changeIncidenceDisplay()-method was probably
  // called by one of the MonthItem objects. So only schedule a reload
  // as event
  setChanges( changes() | IncidencesEdited );
  d->reloadTimer.start( 50 );
}

void MonthView::updateView()
{
  d->view->update();
}

#ifndef QT_NO_WHEELEVENT
void MonthView::wheelEvent( QWheelEvent *event )
{
  // invert direction to get scroll-like behaviour
  if ( event->delta() > 0 ) {
    d->moveStartDate( -1, 0 );
  } else if ( event->delta() < 0 ) {
    d->moveStartDate( 1, 0 );
  }

  // call accept in every case, we do not want anybody else to react
  event->accept();
}
#endif

void MonthView::keyPressEvent( QKeyEvent *event )
{
  if ( event->key() == Qt::Key_PageUp ) {
    d->moveStartDate( 0, -1 );
    event->accept();
  } else if ( event->key() == Qt::Key_PageDown ) {
    d->moveStartDate( 0, 1 );
    event->accept();
  } else if ( processKeyEvent( event ) ) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MonthView::keyReleaseEvent( QKeyEvent *event )
{
  if ( processKeyEvent( event ) ) {
    event->accept();
  } else {
    event->ignore();
  }
}

void MonthView::changeFullView()
{
  bool fullView = d->fullView->isChecked();

  if( fullView ) {
    d->fullView->setIcon( QIcon::fromTheme( QLatin1String("view-restore") ) );
    d->fullView->setToolTip( i18nc( "@info:tooltip",
                                    "Display calendar in a normal size" ) );
  } else {
    d->fullView->setIcon( QIcon::fromTheme( QLatin1String("view-fullscreen") ) );
    d->fullView->setToolTip( i18nc( "@info:tooltip",
                                    "Display calendar in a full window" ) );
  }
  preferences()->setFullViewMonth( fullView );
  preferences()->writeConfig();

  emit fullViewChanged( fullView );
}

void MonthView::moveBackMonth()
{
  d->moveStartDate( 0, -1 );
}

void MonthView::moveBackWeek()
{
  d->moveStartDate( -1, 0 );
}

void MonthView::moveFwdWeek()
{
  d->moveStartDate( 1, 0 );
}

void MonthView::moveFwdMonth()
{
  d->moveStartDate( 0, 1 );
}

void MonthView::showDates( const QDate &start, const QDate &end, const QDate &preferedMonth )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_UNUSED( preferedMonth );
  d->triggerDelayedReload( DatesChanged );
}

QPair<KDateTime,KDateTime> MonthView::actualDateRange( const KDateTime &start,
                                                       const KDateTime &,
                                                       const QDate &preferredMonth ) const
{
  KDateTime dayOne = preferredMonth.isValid() ? KDateTime( preferredMonth ) : start;
  dayOne.setDate( QDate( dayOne.date().year(), dayOne.date().month(), 1 ) );
  const int weekdayCol = ( dayOne.date().dayOfWeek() + 7 - QLocale().firstDayOfWeek() ) % 7;
  KDateTime actualStart = dayOne.addDays( -weekdayCol );
  actualStart.setTime( QTime( 0, 0, 0, 0 ) );
  KDateTime actualEnd = actualStart.addDays( 6 * 7 - 1 );
  actualEnd.setTime( QTime( 23, 59, 59, 99 ) );
  return qMakePair( actualStart, actualEnd );
}

Akonadi::Item::List MonthView::selectedIncidences() const
{
  Akonadi::Item::List selected;
  if ( d->scene->selectedItem() ) {
    IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( d->scene->selectedItem() );
    if ( tmp ) {
      Akonadi::Item incidenceSelected = tmp->akonadiItem();
      if ( incidenceSelected.isValid() ) {
        selected.append( incidenceSelected );
      }
    }
  }
  return selected;
}

void MonthView::reloadIncidences()
{
  if ( changes() == NothingChanged ) {
    return;
  }
  // keep selection if it exists
  Akonadi::Item incidenceSelected;

  MonthItem *itemToReselect = 0;

  if ( IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( d->scene->selectedItem() ) ) {
    d->selectedItemId = tmp->akonadiItem().id();
    d->selectedItemDate = tmp->realStartDate();
    if ( !d->selectedItemDate.isValid() ) {
      return;
    }
  }

  d->scene->resetAll();
  d->mBusyDays.clear();
  // build monthcells hash
  int i = 0;
  for ( QDate date = actualStartDateTime().date();
        date <= actualEndDateTime().date(); date = date.addDays( 1 ) ) {
    d->scene->mMonthCellMap[ date ] = new MonthCell( i, date, d->scene );
    i ++;
  }

  // build global event list
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  const bool colorMonthBusyDays = preferences()->colorMonthBusyDays();

  KCalCore::OccurrenceIterator occurIter( *calendar(), actualStartDateTime(), actualEndDateTime() );
  while ( occurIter.hasNext() ) {
    occurIter.next();

    // Remove the two checks when filtering is done through a proxyModel, when using calendar search
    if ( !preferences()->showTodosMonthView() &&
         occurIter.incidence()->type() == KCalCore::Incidence::TypeTodo ) {
      continue;
    }
    if ( !preferences()->showJournalsMonthView() &&
         occurIter.incidence()->type() == KCalCore::Incidence::TypeJournal ) {
      continue;
    }

    const bool busyDay = colorMonthBusyDays && makesWholeDayBusy( occurIter.incidence() );
    if ( busyDay ) {
      QStringList &list = d->mBusyDays[occurIter.occurrenceStartDate().date()];
      list.append( occurIter.incidence()->uid() );
    }

    const Akonadi::Item item = calendar()->item( occurIter.incidence() );
    if ( !item.isValid() ) {
      continue;
    }
    Q_ASSERT(item.isValid());
    Q_ASSERT(item.hasPayload());
    MonthItem *manager = new IncidenceMonthItem( d->scene,
        calendar(),
        item,
        occurIter.incidence(),
        occurIter.occurrenceStartDate().toTimeSpec( timeSpec ).date() );
    d->scene->mManagerList << manager;
    if ( d->selectedItemId == item.id() &&
        manager->realStartDate() == d->selectedItemDate ) {
      // only select it outside the loop because we are still creating items
      itemToReselect = manager;
    }
  }

  if ( itemToReselect ) {
    d->scene->selectItem( itemToReselect );
  }

  // add holidays
  const QList<QDate> workDays = CalendarSupport::workDays( actualStartDateTime().date(),
                                                           actualEndDateTime().date() );

  for ( QDate date = actualStartDateTime().date();
        date <= actualEndDateTime().date(); date = date.addDays( 1 ) ) {
    // Only call CalendarSupport::holiday() if it's not a workDay, saves come cpu cicles.
    if ( !workDays.contains( date ) ) {
      QStringList holidays( CalendarSupport::holiday( date ) );
      if ( !holidays.isEmpty() ) {
        MonthItem *holidayItem =
          new HolidayMonthItem(
            d->scene, date,
            holidays.join( i18nc( "@item:intext delimiter for joining holiday names", "," ) ) );
        d->scene->mManagerList << holidayItem;
      }
    }
  }

  // sort it
  qSort( d->scene->mManagerList.begin(),
         d->scene->mManagerList.end(),
         MonthItem::greaterThan );

  // build each month's cell event list
  foreach ( MonthItem *manager, d->scene->mManagerList ) {
    for ( QDate date = manager->startDate();
          date <= manager->endDate(); date = date.addDays( 1 ) ) {
      MonthCell *cell = d->scene->mMonthCellMap.value( date );
      if ( cell ) {
        cell->mMonthItemList << manager;
      }
    }
  }

  foreach ( MonthItem *manager, d->scene->mManagerList ) {
    manager->updateMonthGraphicsItems();
    manager->updatePosition();
  }

  foreach ( MonthItem *manager, d->scene->mManagerList ) {
    manager->updateGeometry();
  }

  d->scene->setInitialized( true );
  d->view->update();
  d->scene->update();
}

void MonthView::calendarReset()
{
  qDebug();
  d->triggerDelayedReload( ResourcesChanged );
}

QDate MonthView::averageDate() const
{
  return actualStartDateTime().date().addDays(
    actualStartDateTime().date().daysTo( actualEndDateTime().date() ) / 2 );
}

int MonthView::currentMonth() const
{
  return averageDate().month();
}

bool MonthView::usesFullWindow()
{
  return preferences()->fullViewMonth();
}

bool MonthView::isBusyDay( const QDate &day ) const
{
  return !d->mBusyDays[day].isEmpty();
}

void MonthView::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  Q_ASSERT( cal );

  if ( calendar() ) {
    calendar()->unregisterObserver( d );
  }

  EventView::setCalendar( cal );
  calendar()->registerObserver( d );
}

