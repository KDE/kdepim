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

#include <KGlobal>
#include <KGlobalSettings>

#include <calendarviews/agenda/agendaview.h>
#include <Akonadi/Calendar/ETMCalendar>
#include <KLocale>
#include <KDebug>

using namespace EventViews;

AgendaViewItem::AgendaViewItem(QDeclarativeItem* parent)
  : DeclarativeAkonadiItem( parent )
  , m_view( new AgendaView( QDate(), QDate(),
                            false /*interactive*/,
                            false /*side-by-side*/,
                            0 /*parent*/) )
  , m_currentRange( Week )
{
  // start with the oxygen palette (which is not necessarily the default on all platforms)
  QPalette pal = KGlobalSettings::createApplicationPalette( KGlobal::config() );
  m_view->setPalette( pal );
  m_view->setDateRangeSelectionEnabled( false );

  connect( m_view, SIGNAL(incidenceSelected(Akonadi::Item,QDate)),
           SIGNAL(itemSelected(Akonadi::Item,QDate)) );
  connect( this, SIGNAL(nextItemRequest()), SLOT(gotoNext()) );
  connect( this, SIGNAL(previousItemRequest()), SLOT(gotoPrevious()) );

  setWidget( m_view );
  showRange( QDate::currentDate(), Week );

  preferences()->readConfig();
}

AgendaViewItem::~AgendaViewItem()
{
  preferences()->writeConfig();
  delete m_view;
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
  return m_view->calendar().data();
}

void AgendaViewItem::setCalendar(QObject* calendarObj)
{
  Akonadi::ETMCalendar *cal = qobject_cast<Akonadi::ETMCalendar*>( calendarObj );

  kDebug() << calendarObj << cal;
  if ( cal ) {
    m_view->setCalendar( cal->weakPointer().toStrongRef().dynamicCast<Akonadi::ETMCalendar>() );
    m_view->updateConfig();
  }
}

void AgendaViewItem::showRange( const QDate &date, int range )
{
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

void AgendaViewItem::showToday()
{
  showRange( QDate::currentDate() , m_currentRange );
}


qint64 AgendaViewItem::selectedItemId() const
{
  if ( m_view->selectedIncidences().size() < 1 )
    return -1;
  return m_view->selectedIncidences().first().id();
}

void AgendaViewItem::clearSelection()
{
  m_view->clearSelection();
}

void AgendaViewItem::gotoNext()
{
  const QDate start = (m_currentRange == WorkWeek ? startDate().addDays( 7 )
                                                  : endDate().addDays( 1 ));
  const QDate end = start.addDays( startDate().daysTo( endDate() ) );
  m_view->blockSignals( true );
  m_view->showDates( start, end );
  m_view->clearSelection();
  m_view->blockSignals( false );
}

void AgendaViewItem::gotoPrevious()
{
  const QDate end = (m_currentRange == WorkWeek ? endDate().addDays( -7 )
                                                : startDate().addDays( -1 ));
  const QDate start = end.addDays( - startDate().daysTo( endDate() ) );
  m_view->blockSignals( true );
  m_view->showDates( start, end );
  m_view->clearSelection();
  m_view->blockSignals( false );
}

void AgendaViewItem::setPreferences( const PrefsPtr &preferences )
{
  m_view->setPreferences( preferences );
}

PrefsPtr AgendaViewItem::preferences() const
{
  return m_view->preferences();
}

void AgendaViewItem::updateConfig()
{
  m_view->updateConfig();
}


