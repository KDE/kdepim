/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <stdlib.h>

#include <kdebug.h>
#include <klocale.h>

#include "vcaldrag.h"
#include "vcalformat.h"
#include "icalformat.h"
#include "exceptions.h"
#include "calfilter.h"

#include "calendar.h"

using namespace KCal;

/** \internal */
class AddIncidenceVisitor : public Incidence::Visitor {
  public:
    /** Add incidence to calendar \a calendar. */
    AddIncidenceVisitor(Calendar *calendar) : mCalendar(calendar) {}
    
    bool visit(Event *e) { mCalendar->addEvent(e); return true; }
    bool visit(Todo *t) { mCalendar->addTodo(t); return true; }

  private:
    Calendar *mCalendar;
};

Calendar::Calendar()
{
  mTimeZoneId = QString::fromLatin1("UTC");

  init();
}

Calendar::Calendar(const QString &timeZoneId)
{
  mTimeZoneId = timeZoneId;
  
  init();
}

void Calendar::init()
{
  mObserver = 0;
  mNewObserver = false;

  mModified = false;

  mDndFormat = new VCalFormat;
  
  mFormat = 0;

  mICalFormat = new ICalFormat;

  // Setup default filter, which does nothing
  mDefaultFilter = new CalFilter;
  mFilter = mDefaultFilter;
  mFilter->setEnabled(false);

  // initialize random numbers.  This is a hack, and not
  // even that good of one at that.
//  srandom(time(0L));

  // user information...
  setOwner(i18n("Unknown Name"));
  setEmail(i18n("unknown@nowhere"));

#if 0
  tmpStr = KOPrefs::instance()->mTimeZone;
//  kdDebug(5800) << "Calendar::Calendar(): TimeZone: " << tmpStr << endl;
  int dstSetting = KOPrefs::instance()->mDaylightSavings;
  extern long int timezone;
  struct tm *now;
  time_t curtime;
  curtime = time(0);
  now = localtime(&curtime);
  int hourOff = - ((timezone / 60) / 60);
  if (now->tm_isdst)
    hourOff += 1;
  QString tzStr;
  tzStr.sprintf("%.2d%.2d",
		hourOff, 
		abs((timezone / 60) % 60));

  // if no time zone was in the config file, write what we just discovered.
  if (tmpStr.isEmpty()) {
//    KOPrefs::instance()->mTimeZone = tzStr;
  } else {
    tzStr = tmpStr;
  }
  
  // if daylight savings has changed since last load time, we need
  // to rewrite these settings to the config file.
  if ((now->tm_isdst && !dstSetting) ||
      (!now->tm_isdst && dstSetting)) {
    KOPrefs::instance()->mTimeZone = tzStr;
    KOPrefs::instance()->mDaylightSavings = now->tm_isdst;
  }
  
  setTimeZone(tzStr);
#endif

//  KOPrefs::instance()->writeConfig();
}

Calendar::~Calendar() 
{
  delete mDefaultFilter;
  delete mICalFormat;
  delete mDndFormat;
  delete mFormat;
}

CalFormat *Calendar::calFormat()
{
  return mFormat;
}

ICalFormat *Calendar::iCalFormat()
{
  return mICalFormat;
}

const QString &Calendar::getOwner() const
{
  return mOwner;
}

void Calendar::setOwner(const QString &os)
{
  int i;
  mOwner = os;
  i = mOwner.find(',');
  if (i != -1)
    mOwner = mOwner.left(i);

  setModified( true );
}

void Calendar::setTimeZone(const QString & tz)
{
  bool neg = FALSE;
  int hours, minutes;
  QString tmpStr(tz);

  if (tmpStr.left(1) == "-")
    neg = TRUE;
  if (tmpStr.left(1) == "-" || tmpStr.left(1) == "+")
    tmpStr.remove(0, 1);
  hours = tmpStr.left(2).toInt();
  if (tmpStr.length() > 2) 
    minutes = tmpStr.right(2).toInt();
  else
    minutes = 0;
  mTimeZone = (60*hours+minutes);
  if (neg)
    mTimeZone = -mTimeZone;
  mLocalTime = false;

  setModified( true );
}

QString Calendar::getTimeZoneStr() const 
{
  if (mLocalTime)
    return QString();
  QString tmpStr;
  int hours = abs(mTimeZone / 60);
  int minutes = abs(mTimeZone % 60);
  bool neg = mTimeZone < 0;

  tmpStr.sprintf("%c%.2d%.2d",
		 (neg ? '-' : '+'),
		 hours, minutes);
  return tmpStr;
}

void Calendar::setTimeZone(int tz)
{
  mTimeZone = tz;
  mLocalTime = false;

  setModified( true );
}

int Calendar::getTimeZone() const
{
  return mTimeZone;
}

void Calendar::setTimeZoneId(const QString &id)
{
  mTimeZoneId = id;
  mLocalTime = false;

  setModified( true );
}

QString Calendar::timeZoneId() const
{
  return mTimeZoneId;
}

void Calendar::setLocalTime()
{
  mLocalTime = true;
  mTimeZone = 0;
  mTimeZoneId = "";

  setModified( true );
}

bool Calendar::isLocalTime() const
{
  return mLocalTime;
}

const QString &Calendar::getEmail()
{
  return mOwnerEmail;
}

void Calendar::setEmail(const QString &e)
{
  mOwnerEmail = e;

  setModified( true );
}

void Calendar::setFilter(CalFilter *filter)
{
  mFilter = filter;
}

CalFilter *Calendar::filter()
{
  return mFilter;
}

QPtrList<Event> Calendar::getEventsForDate(const QDate &date,bool sorted)
{
  QPtrList<Event> el = rawEventsForDate(date,sorted);
  mFilter->apply(&el);
  return el;
}

QPtrList<Event> Calendar::getEventsForDate(const QDateTime &qdt)
{
  QPtrList<Event> el = rawEventsForDate(qdt);
  mFilter->apply(&el);
  return el;
}

QPtrList<Event> Calendar::getEvents(const QDate &start,const QDate &end,
                                    bool inclusive)
{
  QPtrList<Event> el = rawEvents(start,end,inclusive);
  mFilter->apply(&el);
  return el;
}


void Calendar::addIncidence(Incidence *i)
{
  AddIncidenceVisitor v(this);

  i->accept(v);
}

QPtrList<Todo> Calendar::getFilteredTodoList()
{
  QPtrList<Todo> tl = getTodoList();
  mFilter->apply(&tl);
  return tl;
}

void Calendar::registerObserver( Observer *observer )
{
  mObserver = observer;
  mNewObserver = true;
}

void Calendar::setModified( bool modified )
{
  if ( modified != mModified || mNewObserver ) {
    mNewObserver = false;
    if ( mObserver ) mObserver->calendarModified( modified, this );
    mModified = modified;
  }
}
