/*
    This file is part of libkcal.

    Copyright (c) 2001-2003 Cornelius Schumacher <schumacher@kde.org>

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

#include "todo.h"

using namespace KCal;

Todo::Todo()
{
  mHasDueDate = false;
  mHasStartDate = false;

  mHasCompletedDate = false;
  mPercentComplete = 0;
}

Todo::Todo(const Todo &t) : Incidence(t)
{
  mDtDue = t.mDtDue;
  mHasDueDate = t.mHasDueDate;
  mHasStartDate = t.mHasStartDate;
  mCompleted = t.mCompleted;
  mHasCompletedDate = t.mHasCompletedDate;
  mPercentComplete = t.mPercentComplete;
  mDtRecurrence = t.mDtRecurrence;
}

Todo::~Todo()
{
}

Todo *Todo::clone()
{
  return new Todo( *this );
}


bool Todo::operator==( const Todo& t2 ) const
{
    return
        static_cast<const Incidence&>(*this) == static_cast<const Incidence&>(t2) &&
        dtDue() == t2.dtDue() &&
        hasDueDate() == t2.hasDueDate() &&
        hasStartDate() == t2.hasStartDate() &&
        completed() == t2.completed() &&
        hasCompletedDate() == t2.hasCompletedDate() &&
        percentComplete() == t2.percentComplete();
}

void Todo::setDtDue(const QDateTime &dtDue, bool first )
{
  //int diffsecs = mDtDue.secsTo(dtDue);

  /*if (mReadOnly) return;
  const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next()) {
    if (alarm->enabled()) {
      alarm->setTime(alarm->time().addSecs(diffsecs));
    }
  }*/
  if( doesRecur() && !first ) {
    mDtRecurrence = dtDue;
  } else {
    mDtDue = dtDue;
    recurrence()->setRecurStart( dtDue );
  }

  if ( doesRecur() && dtDue < recurrence()->recurStart() )
    setDtStart( dtDue );

  //kdDebug(5800) << "setDtDue says date is " << mDtDue.toString() << endl;

  /*const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next())
    alarm->setAlarmStart(mDtDue);*/

  updated();
}

QDateTime Todo::dtDue( bool first ) const
{
  if ( doesRecur() && !first && mDtRecurrence.isValid() )
    return mDtRecurrence;

  return mDtDue;
}

QString Todo::dtDueTimeStr() const
{
  return KGlobal::locale()->formatTime( dtDue(!doesRecur()).time() );
}

QString Todo::dtDueDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(dtDue( !doesRecur() ).date(),shortfmt);
}

QString Todo::dtDueStr() const
{
  return KGlobal::locale()->formatDateTime( dtDue( !doesRecur() ) );
}

bool Todo::hasDueDate() const
{
  return mHasDueDate;
}

void Todo::setHasDueDate(bool f)
{
  if (mReadOnly) return;
  mHasDueDate = f;
  updated();
}


bool Todo::hasStartDate() const
{
  return mHasStartDate;
}

void Todo::setHasStartDate(bool f)
{
  if (mReadOnly) return;

  if ( doesRecur() && !f ) {
    if ( !comments().grep("NoStartDate").count() )
      addComment("NoStartDate"); //TODO: --> custom flag?
  } else {
    QString s("NoStartDate");
    removeComment(s);
  }
  mHasStartDate = f;
  updated();
}

QDateTime Todo::dtStart( bool first ) const
{
  if ( doesRecur() && !first )
    return mDtRecurrence.addDays( dtDue( true ).daysTo( IncidenceBase::dtStart() ) );
  else
    return IncidenceBase::dtStart();
}

void Todo::setDtStart( const QDateTime &dtStart )
{
  if ( doesRecur() )
    recurrence()->setRecurStart( mDtDue );
  IncidenceBase::setDtStart( dtStart );
}

QString Todo::dtStartTimeStr( bool first ) const
{
  return KGlobal::locale()->formatTime(dtStart(first).time());
}

QString Todo::dtStartDateStr(bool shortfmt, bool first) const
{
  return KGlobal::locale()->formatDate(dtStart(first).date(),shortfmt);
}

QString Todo::dtStartStr(bool first) const
{
  return KGlobal::locale()->formatDateTime(dtStart(first));
}

bool Todo::isCompleted() const
{
  if (mPercentComplete == 100) return true;
  else return false;
}

void Todo::setCompleted(bool completed)
{
  if (completed) mPercentComplete = 100;
  else mPercentComplete = 0;
  updated();
}

QDateTime Todo::completed() const
{
  return mCompleted;
}

QString Todo::completedStr() const
{
  return KGlobal::locale()->formatDateTime(mCompleted);
}

void Todo::setCompleted(const QDateTime &completed)
{
  if( !recurTodo() ) {
    mHasCompletedDate = true;
    mPercentComplete = 100;
    mCompleted = completed;
  }
  updated();
}

bool Todo::hasCompletedDate() const
{
  return mHasCompletedDate;
}

int Todo::percentComplete() const
{
  return mPercentComplete;
}

void Todo::setPercentComplete(int v)
{
  mPercentComplete = v;
  updated();
}

void Todo::setDtRecurrence( const QDateTime &dt )
{
  mDtRecurrence = dt;
}

QDateTime Todo::dtRecurrence() const
{
  return mDtRecurrence.isValid() ? mDtRecurrence : mDtDue;
}

bool Todo::recursOn( const QDate &date )
{
  QDate today = QDate::currentDate();
  return ( Incidence::recursOn(date) &&
           !( date < today && mDtRecurrence.date() < today &&
              mDtRecurrence > recurrence()->recurStart() ) );
}

bool Todo::recurTodo()
{
  if ( doesRecur() ) {
    Recurrence *r = recurrence();
    QDateTime endDateTime = r->endDateTime();
    QDateTime nextDate = r->getNextDateTime( dtDue() );

    if ( ( r->duration() == -1 || ( nextDate.isValid() && endDateTime.isValid()
           && nextDate <= endDateTime ) ) ) {
      setDtDue( nextDate );
      while ( !recursAt( dtDue() ) || dtDue() <= QDateTime::currentDateTime() ) {
        setDtDue( r->getNextDateTime( dtDue() ) );
      }

      setCompleted( false );
      setRevision( revision() + 1 );

      return true;
    }
  }

  return false;
}

bool Todo::isOverdue()
{
  bool inPast = doesFloat() ? mDtDue.date() < QDate::currentDate()
                            : mDtDue < QDateTime::currentDateTime();
  return ( inPast && !isCompleted() );
}
