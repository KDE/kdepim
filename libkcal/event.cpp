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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "event.h"

using namespace KCal;

Event::Event() :
  mHasEndDate( false ), mTransparency( Opaque )
{
}

Event::Event(const Event &e) : Incidence(e)
{
  mDtEnd = e.mDtEnd;
  mHasEndDate = e.mHasEndDate;
  mTransparency = e.mTransparency;
}

Event::~Event()
{
//  kdDebug(5800) << "~Event() " << int( this ) << endl;
}

Event *Event::clone()
{
//  kdDebug(5800) << "Event::clone()" << endl;
  return new Event(*this);
}

Event& Event::operator=( const Event &e )
{
  Incidence::operator=( e );
  mDtEnd = e.mDtEnd;
  mHasEndDate = e.mHasEndDate;
  mTransparency = e.mTransparency;
  return *this;
}

bool Event::operator==( const Event& e2 ) const
{
    return
        static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(e2) &&
        dtEnd() == e2.dtEnd() &&
        hasEndDate() == e2.hasEndDate() &&
        transparency() == e2.transparency();
}



void Event::setDtEnd(const QDateTime &dtEnd)
{
  if (mReadOnly) return;

  mDtEnd = dtEnd;

  setHasEndDate(true);
  setHasDuration(false);

  updated();
}

QDateTime Event::dtEnd() const
{
  if (hasEndDate()) return mDtEnd;
  if (hasDuration()) return dtStart().addSecs(duration());

  // It is valid for a VEVENT to be without a DTEND. See RFC2445, Sect4.6.1.
  // Be careful to use Event::dateEnd() as appropriate due to this possibility.
  return dtStart();
}

QDate Event::dateEnd() const
{
  if ( doesFloat() ) return dtEnd().date();
  else return dtEnd().addSecs(-1).date();
}

QString Event::dtEndTimeStr() const
{
  return KGlobal::locale()->formatTime(dtEnd().time());
}

QString Event::dtEndDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtEnd().date(),shortfmt);
}

QString Event::dtEndStr() const
{
  return KGlobal::locale()->formatDateTime(dtEnd());
}

void Event::setHasEndDate(bool b)
{
  mHasEndDate = b;
}

bool Event::hasEndDate() const
{
  return mHasEndDate;
}

bool Event::isMultiDay() const
{
  // End date is non inclusive, so subtract 1 second...
  QDateTime start( dtStart() );
  QDateTime end( dtEnd() );
  if ( ! doesFloat() ) {
    end = end.addSecs(-1);
  }
  bool multi = ( start.date() != end.date() && start <= end );
  return multi;
}

void Event::setTransparency(Event::Transparency transparency)
{
  if (mReadOnly) return;
  mTransparency = transparency;
  updated();
}

Event::Transparency Event::transparency() const
{
  return mTransparency;
}

void Event::setDuration(int seconds)
{
  setHasEndDate(false);
  Incidence::setDuration(seconds);
}

bool Event::slicesInterval( const QDateTime& startDt, const QDateTime& endDt )
{
    QDateTime closestStart = dtStart();
    QDateTime closestEnd = dtEnd();
    if ( doesRecur() ) {
        if ( !recurrence()->timesInInterval( startDt, endDt ).isEmpty() ) {
            // If there is a recurrence in this interval we know already that we slice.
            return true;
        }
        closestStart = recurrence()->getPreviousDateTime( startDt );
        if ( hasEndDate() ) {
            closestEnd = closestStart.addSecs( dtStart().secsTo( dtEnd() ) );
        }
    } else {
        if ( !hasEndDate() && hasDuration() ) {
            closestEnd = closestStart.addSecs( duration() );
        }
    }

    if ( !closestEnd.isValid () ) {
        // All events without an ending still happen if they are
        // started.
        return closestStart <= startDt;
    }

    if ( closestStart <= startDt ) {
        // It starts before the interval and ends after the start of the interval.
        return closestEnd > startDt;
    }

    // Are start and end both in this interval?
    return ( closestStart >= startDt && closestStart <= endDt ) &&
           ( closestEnd >= startDt && closestEnd <= endDt );
}
