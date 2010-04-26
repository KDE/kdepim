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

#include <calendarviews/agenda/agendaview.h>

using namespace CalendarViews;

AgendaViewItem::AgendaViewItem(QDeclarativeItem* parent) :
  DeclarativeAkonadiItem( parent )
{
  m_view = new AgendaView( 0 );
  connect( m_view, SIGNAL(incidenceSelected(Akonadi::Item,QDate)), SIGNAL(itemSelected()) );
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

qint64 AgendaViewItem::selectedItemId() const
{
  if ( m_view->selectedIncidences().size() < 1 )
    return -1;
  return m_view->selectedIncidences().first().id();
}


#include "agendaviewitem.moc"
