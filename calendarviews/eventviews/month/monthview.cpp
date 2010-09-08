/*

  This file is part of KOrganizer.

  Copyright (c) 2008 Bruno Virlet <bruno.virlet@gmail.com>

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

#include "monthview_p.h"
#include "monthscene.h"
#include "monthitem.h"
#include "monthgraphicsitems.h"
#include "prefs.h"

#include <calendarsupport/calendar.h>
#include <calendarsupport/calendarsearch.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/utils.h>

#include <kcalcore/incidence.h>

#include <KIcon>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QDate>
#include <QTimer>

using namespace EventViews;

MonthView::MonthView( QWidget *parent )
  : EventView( new MonthViewPrivate( this ), parent )
{
  Q_D( MonthView );

  d->calendarSearch = new CalendarSupport::CalendarSearch( this );
  d->scene = new MonthScene( this );
  d->view = new MonthGraphicsView( this );
  d->view->setScene( d->scene );
  
  QHBoxLayout *topLayout = new QHBoxLayout( this );
  topLayout->addWidget( d->view );

  QVBoxLayout *rightLayout = new QVBoxLayout( );
  rightLayout->setSpacing( 0 );
  rightLayout->setMargin( 0 );

  // push buttons to the bottom
  rightLayout->addStretch( 1 );

  QToolButton *minusMonth = new QToolButton( this );
  minusMonth->setIcon( KIcon( "arrow-up-double" ) );
  minusMonth->setToolTip( i18n( "Go back one month" ) );
  minusMonth->setAutoRaise( true );
  connect( minusMonth, SIGNAL(clicked()),
           this, SLOT(moveBackMonth()) );

  QToolButton *minusWeek = new QToolButton( this );
  minusWeek->setIcon( KIcon( "arrow-up" ) );
  minusWeek->setToolTip( i18n( "Go back one week" ) );
  minusWeek->setAutoRaise( true );
  connect( minusWeek, SIGNAL(clicked()),
           this, SLOT(moveBackWeek()) );

  QToolButton *plusWeek = new QToolButton( this );
  plusWeek->setIcon( KIcon( "arrow-down" ) );
  plusWeek->setToolTip( i18n( "Go forward one week" ) );
  plusWeek->setAutoRaise( true );
  connect( plusWeek, SIGNAL(clicked()),
           this, SLOT(moveFwdWeek()) );

  QToolButton *plusMonth = new QToolButton( this );
  plusMonth->setIcon( KIcon( "arrow-down-double" ) );
  plusMonth->setToolTip( i18n( "Go forward one month" ) );
  plusMonth->setAutoRaise( true );
  connect( plusMonth, SIGNAL(clicked()),
           this, SLOT(moveFwdMonth()) );

  rightLayout->addWidget( minusMonth );
  rightLayout->addWidget( minusWeek );
  rightLayout->addWidget( plusWeek );
  rightLayout->addWidget( plusMonth );

  topLayout->addLayout( rightLayout );

  connect( d->calendarSearch->model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ),
           this, SLOT( rowsInserted( const QModelIndex&, int, int ) ) );
  connect( d->calendarSearch->model(), SIGNAL( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ),
           this, SLOT( rowsAboutToBeRemoved( const QModelIndex&, int, int ) ) );
  connect( d->calendarSearch->model(), SIGNAL( dataChanged( const QModelIndex&, const QModelIndex& ) ),
           this, SLOT( dataChanged( const QModelIndex&, const QModelIndex& ) ) );
  connect( d->calendarSearch->model(), SIGNAL( modelReset() ), this, SLOT( calendarReset() ) );
  
  connect( d->scene, SIGNAL(showIncidencePopupSignal(Akonadi::Item, QDate)),
           SIGNAL(showIncidencePopup(Akonadi::Item, QDate)) );

  connect( d->scene, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           this, SIGNAL(incidenceSelected(Akonadi::Item,QDate)) );

  connect( d->scene, SIGNAL(newEventSignal(Akonadi::Collection::List)),
           this, SIGNAL(newEventSignal(Akonadi::Collection::List)) );

  connect( &d->reloadTimer, SIGNAL(timeout()), this, SLOT(reloadIncidences()) );
  d->reloadTimer.start( 50 );
  updateConfig();
}

MonthView::~MonthView()
{ }

void MonthView::updateConfig()
{
  Q_D( MonthView );
  CalendarSupport::CalendarSearch::IncidenceTypes types;
  if ( preferences()->showTodosMonthView() ) {
    types |= CalendarSupport::CalendarSearch::Todos;
  }

  if ( preferences()->showJournalsMonthView() ) {
    types |= CalendarSupport::CalendarSearch::Journals;
  }

  types |= CalendarSupport::CalendarSearch::Events;
  d->calendarSearch->setIncidenceTypes( types );
  d->scene->update();
}

int MonthView::currentDateCount() const
{
  return actualStartDateTime().date().daysTo( actualEndDateTime().date() );
}

DateList MonthView::selectedIncidenceDates() const
{
  Q_D( const MonthView );
  DateList list;
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
  Q_D( const MonthView );
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

void MonthView::setDateRange( const KDateTime &start, const KDateTime &end )
{
  Q_D( MonthView );
  EventView::setDateRange( start, end );
  d->calendarSearch->setStartDate( d->actualStartDateTime );
  d->calendarSearch->setEndDate( d->actualEndDateTime );
}
  
bool MonthView::eventDurationHint( QDateTime &startDt, QDateTime &endDt, bool &allDay ) const
{
  Q_D( const MonthView );
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
  QTimer::singleShot( 0, this, SLOT(reloadIncidences()) );
}

void MonthView::updateView()
{
  Q_D( MonthView );
  d->view->update();
}

#ifndef QT_NO_WHEELEVENT
void MonthView::wheelEvent( QWheelEvent *event )
{
  Q_D( MonthView );
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
  Q_D( MonthView );
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

void MonthView::moveBackMonth()
{
  Q_D( MonthView );
  d->moveStartDate( 0, -1 );
}

void MonthView::moveBackWeek()
{
  Q_D( MonthView );
  d->moveStartDate( -1, 0 );
}

void MonthView::moveFwdWeek()
{
  Q_D( MonthView );
  d->moveStartDate( 1, 0 );
}

void MonthView::moveFwdMonth()
{
  Q_D( MonthView );
  d->moveStartDate( 0, 1 );
}

void MonthView::showDates( const QDate &start, const QDate &end )
{
  Q_UNUSED( start );
  Q_UNUSED( end );
  Q_D( MonthView );
  d->triggerDelayedReload();
}

QPair<KDateTime,KDateTime> MonthView::actualDateRange( const KDateTime &start,
                                                       const KDateTime & ) const {
  KDateTime dayOne( start );
  dayOne.setDate( QDate( start.date().year(), start.date().month(), 1 ) );
  const int weekdayCol = ( dayOne.date().dayOfWeek() + 7 - KGlobal::locale()->weekStartDay() ) % 7;
  KDateTime actualStart = dayOne.addDays( -weekdayCol );
  actualStart.setTime( QTime( 0, 0, 0, 0 ) );
  KDateTime actualEnd = actualStart.addDays( 6 * 7 - 1 );
  actualEnd.setTime( QTime( 23, 59, 59, 99 ) );
  return qMakePair( actualStart, actualEnd );
}

Akonadi::Item::List MonthView::selectedIncidences() const
{
  Q_D( const MonthView );
  Akonadi::Item::List selected;
  if ( d->scene->selectedItem() ) {
    IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( d->scene->selectedItem() );
    if ( tmp ) {
      Akonadi::Item incidenceSelected = tmp->incidence();
      if ( incidenceSelected.isValid() ) {
        selected.append( incidenceSelected );
      }
    }
  }
  return selected;
}

void MonthView::reloadIncidences()
{
  Q_D( MonthView );
  // keep selection if it exists
  Akonadi::Item incidenceSelected;

  MonthItem *itemToReselect = 0;

  if ( IncidenceMonthItem *tmp = qobject_cast<IncidenceMonthItem *>( d->scene->selectedItem() ) ) {
    d->selectedItemId = tmp->incidence().id();
    d->selectedItemDate = tmp->realStartDate();
    if ( !d->selectedItemDate.isValid() ) {
      return;
    }
  }

  d->scene->resetAll();
  // build monthcells hash
  int i = 0;
  for ( QDate date = actualStartDateTime().date(); date <= actualEndDateTime().date(); date = date.addDays( 1 ) ) {
    d->scene->mMonthCellMap[ date ] = new MonthCell( i, date, d->scene );
    i ++;
  }

  // build global event list
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  const Akonadi::Item::List incidences = CalendarSupport::itemsFromModel( d->calendarSearch->model() );

  foreach ( const Akonadi::Item &aitem, incidences ) {
    const Incidence::Ptr incidence = CalendarSupport::incidence( aitem );

    DateTimeList dateTimeList;

    if ( incidence->recurs() ) {
      // Get a list of all dates that the recurring event will happen
      dateTimeList = incidence->recurrence()->timesInInterval(
        actualStartDateTime(), actualEndDateTime() );
    } else {
      KDateTime dateToAdd;

      if ( Todo::Ptr todo = CalendarSupport::todo( aitem ) ) {
        if ( todo->hasDueDate() ) {
          dateToAdd = todo->dtDue();
        }
      } else {
        dateToAdd = incidence->dtStart();
      }

      if ( dateToAdd >= actualStartDateTime() &&
           dateToAdd <= actualEndDateTime() ) {
        dateTimeList += dateToAdd;
      }

    }
    DateTimeList::const_iterator t;
    for ( t = dateTimeList.constBegin(); t != dateTimeList.constEnd(); ++t ) {
      MonthItem *manager = new IncidenceMonthItem( d->scene,
                                                   aitem,
                                                   t->toTimeSpec( timeSpec ).date() );
      d->scene->mManagerList << manager;
      if ( d->selectedItemId == aitem.id() &&
           manager->realStartDate() == d->selectedItemDate ) {
        // only select it outside the loop because we are still creating items
        itemToReselect = manager;
      }
    }
  }

  if ( itemToReselect ) {
    d->scene->selectItem( itemToReselect );
  }

  // add holidays
  for ( QDate date = actualStartDateTime().date(); date <= actualEndDateTime().date(); date = date.addDays( 1 ) ) {
    QStringList holidays( CalendarSupport::holiday( date ) );
    if ( !holidays.isEmpty() ) {
      MonthItem *holidayItem =
        new HolidayMonthItem(
          d->scene, date,
          holidays.join( i18nc( "delimiter for joining holiday names", "," ) ) );
      d->scene->mManagerList << holidayItem;
    }
  }

  // sort it
  qSort( d->scene->mManagerList.begin(),
         d->scene->mManagerList.end(),
         MonthItem::greaterThan );

  // build each month's cell event list
  foreach ( MonthItem *manager, d->scene->mManagerList ) {
    for ( QDate date = manager->startDate(); date <= manager->endDate(); date = date.addDays( 1 ) ) {
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
  Q_D( MonthView );
  kDebug();
  d->triggerDelayedReload();
}

void MonthView::dataChanged( const QModelIndex& topLeft, const QModelIndex& bottomRight )
{
  Q_ASSERT( topLeft.parent() == bottomRight.parent() );
  Q_D( MonthView );
  incidencesChanged( CalendarSupport::itemsFromModel( d->calendarSearch->model(), topLeft.parent(),
                    topLeft.row(), bottomRight.row() ) );
}

void MonthView::rowsInserted( const QModelIndex& parent, int start, int end )
{
  Q_D( MonthView );
  incidencesAdded( CalendarSupport::itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

void MonthView::rowsAboutToBeRemoved( const QModelIndex& parent, int start, int end )
{
  Q_D( MonthView );
  incidencesAboutToBeRemoved( CalendarSupport::itemsFromModel( d->calendarSearch->model(), parent, start, end ) );
}

void MonthView::incidencesAdded( const Akonadi::Item::List &incidences )
{
  Q_D( MonthView );
  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  Q_FOREACH ( const Akonadi::Item &i, incidences ) {
    kDebug() << "item added: " << CalendarSupport::incidence( i )->summary();
  }
  d->triggerDelayedReload();
}

void MonthView::incidencesAboutToBeRemoved( const Akonadi::Item::List &incidences )
{
  Q_D( MonthView );
  Q_FOREACH ( const Akonadi::Item &i, incidences ) {
    kDebug() << "item removed: " << CalendarSupport::incidence( i )->summary();
  }
  d->triggerDelayedReload();
}

void MonthView::incidencesChanged( const Akonadi::Item::List &incidences )
{
  Q_D( MonthView );
  Q_FOREACH ( const Akonadi::Item &i, incidences ) {
    kDebug() << "item changed: " << CalendarSupport::incidence( i )->summary();
  }
  d->triggerDelayedReload();
}

QDate MonthView::averageDate() const
{
  return actualStartDateTime().date().addDays( actualStartDateTime().date().daysTo( actualEndDateTime().date() ) / 2 );
}

int MonthView::currentMonth() const
{
  return averageDate().month();
}

bool MonthView::usesFullWindow()
{
  return preferences()->fullViewMonth();
}

// CalPrinterBase::PrintType MonthView::printType()
// {
//   return CalPrinterBase::Month;
// }
