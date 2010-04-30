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

#include "agendaviewitem.h"

#include <akonadi/kcal/calendar.h>
#include <calendarviews/agenda/agendaview.h>

using namespace EventViews;

AgendaViewItem::AgendaViewItem(QDeclarativeItem* parent) :
  DeclarativeAkonadiItem( parent )
{
  m_view = new AgendaView( 0 );
  connect( m_view, SIGNAL(incidenceSelected(Akonadi::Item,QDate)), SIGNAL(itemSelected()) );
  connect( this, SIGNAL(nextItemRequest()), SLOT(gotoNext()) );
  connect( this, SIGNAL(previousItemRequest()), SLOT(gotoPrevious()) );
  setWidget( m_view );
}

AgendaViewItem::~AgendaViewItem()
{
}

QDate AgendaViewItem::startDate() const
{
  return QDate( m_view->startDate() );
}

void AgendaViewItem::setStartDate(const QDate& startDate)
{
  kDebug() << startDate;
  if ( startDate.isValid() )
    m_view->showDates( startDate, endDate() );
}

QDate AgendaViewItem::endDate() const
{
  return QDate( m_view->endDate() );
}

void AgendaViewItem::setEndDate(const QDate& endDate)
{
  kDebug() << endDate;
  if ( endDate.isValid() )
    m_view->showDates( startDate(), endDate );
}

QObject* AgendaViewItem::calendar() const
{
  return m_view->calendar();
}

void AgendaViewItem::setCalendar(QObject* calendarObj)
{
  Akonadi::Calendar* cal = qobject_cast<Akonadi::Calendar*>( calendarObj );
  kDebug() << calendarObj << cal;
  if ( cal ) {
    m_view->setCalendar( cal );
    m_view->updateConfig();
  }
}

void AgendaViewItem::showRange( const QDate &date, int range )
{
  Q_ASSERT( range >= 0 && range <= 3 );

  switch( Range( range ) ) {
  case Day:
    m_view->showDates( date, date );
    break;
  case Week:
    // Todo: Take in account sunday or monday as first day of week.
    m_view->showDates( date.addDays( - date.dayOfWeek() ), date.addDays( 6 - date.dayOfWeek() ) );
    break;
  case Month:
    m_view->showDates( date.addDays( - date.day() + 1 ), date.addDays( date.daysInMonth() - date.day() ) );
    break;
  }
}

qint64 AgendaViewItem::selectedItemId() const
{
  if ( m_view->selectedIncidences().size() < 1 )
    return -1;
  return m_view->selectedIncidences().first().id();
}

void AgendaViewItem::gotoNext()
{
  const QDate start = endDate().addDays( 1 );
  const QDate end = start.addDays( startDate().daysTo( endDate() ) );
  kDebug() << start << end;
  m_view->showDates( start, end );
}

void AgendaViewItem::gotoPrevious()
{
  const QDate end = startDate().addDays( - 1 );
  const QDate start = end.addDays( - startDate().daysTo( endDate() ) );
  kDebug() << start << end;
  m_view->showDates( start, end );

}

#include "agendaviewitem.moc"
