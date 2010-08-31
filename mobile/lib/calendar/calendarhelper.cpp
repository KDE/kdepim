/*
    Copyright (C) 2010 Artur Duque de Souza <asouza@kde.org>

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

#include "calendarhelper.h"

#define MAX_DAYS_ON_WIDGET 35
#define MAX_WEEKS_ON_WIDGET 5

#include <KDebug>


CalendarHelper::CalendarHelper( QObject *parent )
    : QObject( parent )
{
    //setDate(QDateTime::currentDateTime());
    setDate(QDateTime(QDate(2010, 8, 10)));
}

CalendarHelper::~CalendarHelper()
{
}

QDateTime CalendarHelper::date() const
{
  return m_original;
}

void CalendarHelper::setDate( const QDateTime datetime )
{
  m_original = datetime;
  m_day = datetime.date().day();
  m_month = datetime.date().month();
  m_year = datetime.date().year();
  m_daysInMonth = m_original.date().daysInMonth();

  // calculate the offsets for day and week
  QDate firstDay = QDate(m_year, m_month, 1);
  m_offset = firstDay.dayOfWeek();
  m_weekOffset = firstDay.weekNumber();
  emit dateChanged();
}

int CalendarHelper::day() const
{
  return m_day;
}

void CalendarHelper::setDay( const int day )
{
  if ( m_day == day )
    return;

  QDate newDate(m_year, m_month, day);
  if ( !newDate.isValid() )
    return;

  m_original.setDate( newDate );
  m_day = day;
  emit dayChanged();
}

QString CalendarHelper::monthName() const
{
  return QDate::longMonthName( m_month );
}

int CalendarHelper::month() const
{
  return m_month;
}

void CalendarHelper::setMonth( const int month )
{
  if ( m_month == month )
    return;

  QDate newDate(m_year, month, m_day);
  if ( !newDate.isValid() )
    return;

  m_original.setDate( newDate );
  m_month = month;
  m_daysInMonth = m_original.date().daysInMonth();
  emit monthChanged();
}

int CalendarHelper::year() const
{
  return m_year;
}

void CalendarHelper::setYear( const int year )
{
  if ( m_year == year )
      return;

  QDate newDate(year, m_month, m_day);
  if ( !newDate.isValid() )
    return;

  m_original.setDate( newDate );
  m_year = year;
  emit yearChanged();
}

QString CalendarHelper::dayForPosition( const int pos ) const
{
  int res = (pos - m_offset);

  // out of range
  if ( pos < 1 || pos > MAX_DAYS_ON_WIDGET || res < 1)
    return QString();

  // if the position is the firsts days (0 to m_offset)
  if ( pos < m_offset )
    return QString::number(pos);

  // the current day
  if ( pos == m_offset ) {
    const QString rpos = QString::number(pos);
    emit activeDay( rpos );
    return rpos;
  }

  if ( res >= m_daysInMonth )
    return QString();

  return QString::number(res);
}

int CalendarHelper::weekForPosition( const int pos ) const
{
  // out of range
  if ( pos < 1 || pos > MAX_WEEKS_ON_WIDGET )
      return -1;

  // if the position is the first week
  if (pos == 1)
      return m_weekOffset;

  // for all other weeks do the math
  return pos + m_weekOffset - 1;
}

#include "calendarhelper.moc"
