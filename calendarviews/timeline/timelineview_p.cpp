/*
  Copyright (c) 2007 Till Adam <adam@kde.org>
  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Copyright (c) 2010 Andras Mantia <andras@kdab.com>

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

#include "timelineview_p.h"
#include "timelineitem.h"

#include <kdgantt2/kdganttgraphicsview.h>

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/collectionselection.h>
#include <calendarsupport/utils.h>
#include <calendarsupport/kcalprefs.h>
#include <Akonadi/Calendar/IncidenceChanger>
#include <KCalCore/OccurrenceIterator>

#include <QStandardItemModel>
#include <QResizeEvent>
#include <QSplitter>
#include <QTreeWidget>

using namespace KCalCore;
using namespace EventViews;

TimelineView::Private::Private( TimelineView *parent ) :
  q( parent )
{
}

TimelineView::Private::~Private()
{
}

void TimelineView::Private::splitterMoved()
{
  mLeftView->setColumnWidth( 0, mLeftView->width() );
}

void TimelineView::Private::itemSelected( const QModelIndex &index )
{
  TimelineSubItem *tlitem =
    dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>(
                                       mGantt->model() )->item( index.row(), index.column() ) );
  if ( tlitem ) {
    emit q->incidenceSelected( tlitem->incidence(), tlitem->originalStart().date() );
  }
}

void TimelineView::Private::itemDoubleClicked( const QModelIndex &index )
{
  TimelineSubItem *tlitem =
    dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>(
                                       mGantt->model() )->item( index.row(), index.column() ) );
  if ( tlitem ) {
    emit q->editIncidenceSignal( tlitem->incidence() );
  }
}

void TimelineView::Private::contextMenuRequested( const QPoint &point )
{
  QPersistentModelIndex index = mGantt->indexAt( point );
  // mHintDate = QDateTime( mGantt->getDateTimeForCoordX( QCursor::pos().x(), true ) );
  TimelineSubItem *tlitem =
    dynamic_cast<TimelineSubItem *>( static_cast<QStandardItemModel*>(
                                       mGantt->model() )->item( index.row(), index.column() ) );
  if ( !tlitem ) {
    emit q->showNewEventPopupSignal();
    mSelectedItemList = Akonadi::Item::List();
  } else {
    emit q->showIncidencePopupSignal(
      tlitem->incidence(),
      CalendarSupport::incidence( tlitem->incidence() )->dtStart().date() );

    mSelectedItemList << tlitem->incidence();
  }
}

//slot
void TimelineView::Private::newEventWithHint( const QDateTime &dt )
{
  mHintDate = dt;
  emit q->newEventSignal( dt );
}

TimelineItem *TimelineView::Private::calendarItemForIncidence( const Akonadi::Item &incidence )
{
  Akonadi::ETMCalendar::Ptr calres = q->calendar();
  TimelineItem *item = 0;
  if ( !calres ) {
    item = mCalendarItemMap.value( -1 );
  } else {
    item = mCalendarItemMap.value( incidence.parentCollection().id() );
  }
  return item;
}

void TimelineView::Private::insertIncidence( const Akonadi::Item &aitem, const QDate &day )
{
  const Incidence::Ptr incidence = CalendarSupport::incidence( aitem );
  //kDebug() << "Item " << aitem.id() << " parentcollection: " << aitem.parentCollection().id();
  TimelineItem *item = calendarItemForIncidence( aitem );
  if ( !item ) {
    kWarning() << "Help! Something is really wrong here!";
    return;
  }

  if ( incidence->recurs() ) {
    KCalCore::OccurrenceIterator occurIter( *(q->calendar()), incidence, KDateTime( day, QTime( 0, 0 ,0 ) ), KDateTime( day, QTime( 23, 59, 59 ) ) );
    while ( occurIter.hasNext() ) {
      occurIter.next();
      const Akonadi::Item akonadiItem = q->calendar()->item( occurIter.incidence() );
      const KDateTime startOfOccurrence = occurIter.occurrenceStartDate();
      const KDateTime endOfOccurrence = occurIter.incidence()->endDateForStart( startOfOccurrence );
      const KDateTime::Spec spec = CalendarSupport::KCalPrefs::instance()->timeSpec();
      item->insertIncidence( akonadiItem, startOfOccurrence.toTimeSpec( spec ),  endOfOccurrence.toTimeSpec( spec ) );
    }
  } else {
    if ( incidence->dtStart().date() == day ||
         incidence->dtStart().date() < mStartDate ) {
      item->insertIncidence( aitem );
    }
  }
}

void TimelineView::Private::insertIncidence( const Akonadi::Item &incidence )
{
  const Event::Ptr event = CalendarSupport::event( incidence );
  if ( !event ) {
    return;
  }

  if ( event->recurs() ) {
    insertIncidence( incidence, QDate() );
  }

  KDateTime::Spec timeSpec = CalendarSupport::KCalPrefs::instance()->timeSpec();
  for ( QDate day = mStartDate; day <= mEndDate; day = day.addDays( 1 ) ) {
    KCalCore::Event::List events = q->calendar()->events( day,
                                                          timeSpec,
                                                          KCalCore::EventSortStartDate,
                                                          KCalCore::SortDirectionAscending );
    if ( events.contains( event ) ) {
      //PENDING(AKONADI_PORT) check if correct. also check the original if,
      //was inside the for loop (unnecessarily)
      foreach( const KCalCore::Event::Ptr &i, events ) {
        Akonadi::Item item = q->calendar()->item( i );
        insertIncidence( item, day );
      }
    }
  }
}

void TimelineView::Private::removeIncidence( const Akonadi::Item &incidence )
{
  TimelineItem *item = calendarItemForIncidence( incidence );
  if ( item ) {
    item->removeIncidence( incidence );
  } else {
#if 0 //AKONADI_PORT_DISABLED
    // try harder, the incidence might already be removed from the resource
    typedef QMap<QString, KOrg::TimelineItem *> M2_t;
    typedef QMap<KCalCore::ResourceCalendar *, M2_t> M1_t;
    for ( M1_t::ConstIterator it1 = d->mCalendarItemMap.constBegin();
          it1 != mCalendarItemMap.constEnd(); ++it1 ) {
      for ( M2_t::ConstIterator it2 = it1.value().constBegin();
            it2 != it1.value().constEnd(); ++it2 ) {
        if ( it2.value() ) {
          it2.value()->removeIncidence( incidence );
        }
      }
    }
#endif
  }
}

void TimelineView::Private::itemChanged( QStandardItem *item )
{
  TimelineSubItem *tlit = dynamic_cast<TimelineSubItem *>( item );
  if ( !tlit ) {
    return;
  }

  const Akonadi::Item i = tlit->incidence();
  const Incidence::Ptr inc = CalendarSupport::incidence( i );

  KDateTime newStart( tlit->startTime() );
  if ( inc->allDay() ) {
    newStart = KDateTime( newStart.date() );
  }

  int delta = tlit->originalStart().secsTo( newStart );
  inc->setDtStart( inc->dtStart().addSecs( delta ) );
  int duration = tlit->startTime().secsTo( tlit->endTime() );
  int allDayOffset = 0;
  if ( inc->allDay() ) {
    int secsPerDay = 60 * 60 * 24;
    duration /= secsPerDay;
    duration *= secsPerDay;
    allDayOffset = secsPerDay;
    duration -= allDayOffset;
    if ( duration < 0 ) {
      duration = 0;
    }
  }
  inc->setDuration( duration );
  TimelineItem *parent = tlit->parent();
  parent->moveItems( i, tlit->originalStart().secsTo( newStart ), duration + allDayOffset );
}

#include "timelineview_p.moc"
