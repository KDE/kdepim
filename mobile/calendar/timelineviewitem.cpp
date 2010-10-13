/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#include "timelineviewitem.h"

#include <KGlobal>
#include <KGlobalSettings>

#include <calendarsupport/calendar.h>
#include <calendarviews/eventviews/timeline/timelineview.h>
#include <KLocale>

using namespace EventViews;

// TODO: reuse code with AgendaViewItem and MonthViewItem once the dust has settled

TimelineViewItem::TimelineViewItem(QDeclarativeItem* parent)
  : DeclarativeAkonadiItem( parent )
  , m_view( new TimelineView() )
  , m_currentRange( Week )
{
  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );
  m_view->setPalette( pal );
  m_view->setDateRangeSelectionEnabled( false );

  connect( m_view, SIGNAL(incidenceSelected(Akonadi::Item, QDate)),
           SIGNAL(itemSelected()) );
  connect( m_view, SIGNAL(incidenceSelected(Akonadi::Item, QDate)),
           SIGNAL(itemSelected(Akonadi::Item, QDate)) );
  connect( this, SIGNAL(nextItemRequest()), SLOT(gotoNext()) );
  connect( this, SIGNAL(previousItemRequest()), SLOT(gotoPrevious()) );

  setWidget( m_view );
  showRange( QDate::currentDate(), Week );
}

TimelineViewItem::~TimelineViewItem()
{
  delete m_view;
}

QDate TimelineViewItem::startDate() const
{
  return QDate( m_view->startDate() );
}

void TimelineViewItem::setStartDate(const QDate& startDate)
{
  kDebug() << startDate;
  if ( startDate.isValid() )
    m_view->showDates( startDate, endDate() );
}

QDate TimelineViewItem::endDate() const
{
  return QDate( m_view->endDate() );
}

void TimelineViewItem::setEndDate(const QDate& endDate)
{
  kDebug() << endDate;
  if ( endDate.isValid() )
    m_view->showDates( startDate(), endDate );
}

QObject* TimelineViewItem::calendar() const
{
  return m_view->calendar();
}

void TimelineViewItem::setCalendar(QObject* calendarObj)
{
  CalendarSupport::Calendar* cal = qobject_cast<CalendarSupport::Calendar*>( calendarObj );
  kDebug() << calendarObj << cal;
  if ( cal ) {
    m_view->setCalendar( cal );
    m_view->updateConfig();
  }
}

void TimelineViewItem::showRange( const QDate &date, int range )
{
  if ( !m_view->calendar() ) return;

  Q_ASSERT( range >= 0 && range <= LastRange );

  m_currentRange = Range( range );
  switch( m_currentRange ) {
  case Day: {
    m_view->showDates( date, date );
    break;
  }
  case Week: {
    int weekStartDay = KGlobal::locale()->weekStartDay();
    if ( weekStartDay > date.dayOfWeek() )
      weekStartDay = weekStartDay - 7;
    m_view->showDates( date.addDays( weekStartDay - date.dayOfWeek() ), date.addDays( weekStartDay + 6 - date.dayOfWeek() ) );
    break;
  }
  case WorkWeek: {
    int workingWeekStartDay = KGlobal::locale()->workingWeekStartDay();
    int workingWeekEndDay = KGlobal::locale()->workingWeekEndDay();
    m_view->showDates( date.addDays( workingWeekStartDay - date.dayOfWeek() ), date.addDays( workingWeekEndDay - date.dayOfWeek() ) );
    break;
  }
  case Next3Days: {
    m_view->showDates( date, date.addDays( 3 ) );
    break;
  }
  default:;
  }
}

qint64 TimelineViewItem::selectedItemId() const
{
  if ( m_view->selectedIncidences().size() < 1 )
    return -1;
  return m_view->selectedIncidences().first().id();
}

void TimelineViewItem::clearSelection()
{
  m_view->clearSelection();
}

void TimelineViewItem::gotoNext()
{
  const QDate start = endDate().addDays( 1 );
  const QDate end = start.addDays( startDate().daysTo( endDate() ) );
  m_view->blockSignals( true );
  m_view->showDates( start, end );
  m_view->clearSelection();
  m_view->blockSignals( false );
}

void TimelineViewItem::gotoPrevious()
{
  const QDate end = startDate().addDays( - 1 );
  const QDate start = end.addDays( - startDate().daysTo( endDate() ) );
  m_view->blockSignals( true );
  m_view->showDates( start, end );
  m_view->clearSelection();
  m_view->blockSignals( false );
}

#include "timelineviewitem.moc"
