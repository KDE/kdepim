/*
    This file is part of libkcal.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include <kdebug.h>

#include "freebusy.h"

using namespace KCal;

FreeBusy::FreeBusy()
{
}

FreeBusy::FreeBusy(const QDateTime &start, const QDateTime &end)
{
  setDtStart(start);
  setDtEnd(end);
}

FreeBusy::FreeBusy( Calendar *calendar, const QDateTime &start, const QDateTime &end )
{
  kdDebug() << "FreeBusy::FreeBusy" << endl;
  mCalendar = calendar;

  setDtStart(start);
  setDtEnd(end);

  // Get all the events in the calendar
  Event::List eventList = mCalendar->events();

  int extraDays, i, x, duration;
  duration = start.daysTo(end);
  QDate day;
  QDateTime tmpStart;
  QDateTime tmpEnd;
  // Loops through every event in the calendar
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *event = *it;

    // This whole for loop is for recurring events, it loops through 
    // each of the days of the freebusy request

    // First check if this is transparent. If it is, it shouldn't be in the
    // freebusy list
    if ( event->transparency() == Event::Transparent )
      // Transparent
      continue;

    for(i=0; i<=duration; i++) {
      day=(start.addDays(i).date());
      tmpStart.setDate(day);
      tmpEnd.setDate(day);

      if( (*(event->recurrence())).doesRecur() ) {
        if ( event->isMultiDay() ) {
          extraDays = event->dtStart().date().daysTo(event->dtEnd().date());
          for (x=0; x<=extraDays; x++) {
            if ( event->recursOn(day.addDays(-x))) {
              tmpStart.setDate(day.addDays(-x));
              tmpStart.setTime(event->dtStart().time());
              tmpEnd=tmpStart.addSecs( (event->duration()) );

              addLocalPeriod( tmpStart, tmpEnd );
              break;
            }
          }
        } else {
          if (event->recursOn(day)) {
            tmpStart.setTime(event->dtStart().time());
            tmpEnd.setTime(event->dtEnd().time());

            addLocalPeriod (tmpStart, tmpEnd);
          }
        }
      }
    
    }
    // Non-recurring events
    addLocalPeriod(event->dtStart(), event->dtEnd());
  }

  sortList();
}

FreeBusy::~FreeBusy()
{
}

bool FreeBusy::setDtEnd( const QDateTime &end )
{
  mDtEnd = end;
  return true;
}

QDateTime FreeBusy::dtEnd() const
{
  return mDtEnd;
}

QValueList<Period> FreeBusy::busyPeriods() const
{
  return mBusyPeriods;
}

bool FreeBusy::addLocalPeriod(const QDateTime &eventStart, const QDateTime &eventEnd ) {
  QDateTime tmpStart;
  QDateTime tmpEnd;

  //Check to see if the start *or* end of the event is
  //between the start and end of the freebusy dates.
  if (!((((this->dtStart()).secsTo(eventStart)>=0)&&(eventStart.secsTo(this->dtEnd())>=0)) 
    ||(((this->dtStart()).secsTo(eventEnd) >= 0)&&(eventEnd.secsTo(this->dtEnd()) >= 0))))
    return false;

  if ( eventStart.secsTo(this->dtStart())>=0) {
    tmpStart = this->dtStart();
  } else {
    tmpStart = eventStart;
  }

  if ( eventEnd.secsTo(this->dtEnd())<=0 ) {
    tmpEnd = this->dtEnd();
  } else {
    tmpEnd = eventEnd;
  }  

  Period p(tmpStart, tmpEnd);  
  mBusyPeriods.append( p );

  return true;
}

FreeBusy::FreeBusy(QValueList<Period> busyPeriods)
{
  mBusyPeriods = busyPeriods;
}

void FreeBusy::sortList()
{
  typedef QValueList<Period> PeriodList;

  PeriodList::Iterator tmpPeriod, earlyPeriod;
  PeriodList sortedList;
  QDateTime earlyTime;

  while( mBusyPeriods.count() > 0 ) {
    earlyTime=(*mBusyPeriods.begin()).start();
    for (tmpPeriod=mBusyPeriods.begin(); tmpPeriod!=mBusyPeriods.end(); tmpPeriod++) {
      if (earlyTime.secsTo((*tmpPeriod).start()) <= 0) {
        earlyTime=(*tmpPeriod).start();
        earlyPeriod=tmpPeriod;
      }
    } 
    //Move tmpPeriod to sortedList
    Period tmpPeriod( (*earlyPeriod).start(), (*earlyPeriod).end() );
    sortedList.append( tmpPeriod );
    mBusyPeriods.remove( earlyPeriod );
  }
  mBusyPeriods=sortedList;
}

void FreeBusy::addPeriod(const QDateTime &start, const QDateTime &end)
{
  Period p(start, end);
  mBusyPeriods.append( p );

  sortList();
}
