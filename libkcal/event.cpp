// $Id$

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "event.h"

using namespace KCal;

Event::Event() :
  mHasEndDate(false), mTransparency(0) 
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
}

Event *Event::clone()
{
  kdDebug() << "Event::clone()" << endl;
  return new Event(*this);
}

void Event::setDtEnd(const QDateTime &dtEnd)
{  
  if (mReadOnly) return;

  mDtEnd = dtEnd;

  setHasEndDate(true);
  setHasDuration(false);
  
  emit eventUpdated(this);
}

QDateTime Event::dtEnd() const
{
  if (hasEndDate()) return mDtEnd;
  if (hasDuration()) return dtStart().addSecs(duration());

  kdDebug() << "Warning! Event '" << summary()
            << "' does have neither end date nor duration." << endl;
  return QDateTime();
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


void Event::setTransparency(int transparency)
{
  if (mReadOnly) return;
  mTransparency = transparency;
  emit eventUpdated(this);
}

int Event::transparency() const
{
  return mTransparency;
}

void Event::setDuration(int seconds)
{
  setHasEndDate(false);
  Incidence::setDuration(seconds);
}
