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
  kdDebug(5800) << "Event::clone()" << endl;
  return new Event(*this);
}

bool Event::operator==( const Event& e2 ) const
{
    return
        static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(e2) &&
        ( !hasEndDate() || ( dtEnd() == e2.dtEnd() ) ) &&
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

  kdDebug(5800) << "Warning! Event '" << summary()
            << "' does have neither end date nor duration." << endl;
  return dtStart();
}

QString Event::dtEndTimeStr() const
{
  return KGlobal::locale()->formatTime(mDtEnd.time());
}

QString Event::dtEndDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(mDtEnd.date(),shortfmt);
}

QString Event::dtEndStr() const
{
  return KGlobal::locale()->formatDateTime(mDtEnd);
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
  bool multi = !(dtStart().date() == dtEnd().date());
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
