// $Id$

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>

#include "event.h"

using namespace KCal;

Event::Event()
{
  mTransparency = 0;
}

Event::Event(const Event &e) : Incidence(e)
{
  mDtEnd = e.mDtEnd;
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
  emit eventUpdated(this);
}

const QDateTime &Event::dtEnd() const
{
  return mDtEnd;
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


