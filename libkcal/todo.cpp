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

#include "todo.h"

using namespace KCal;

Todo::Todo()
{
//  mStatus = TENTATIVE;

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

void Todo::setDtDue(const QDateTime &dtDue)
{
  //int diffsecs = mDtDue.secsTo(dtDue);

  /*if (mReadOnly) return;
  const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next()) {
    if (alarm->enabled()) {
      alarm->setTime(alarm->time().addSecs(diffsecs));
    }
  }*/
  mDtDue = dtDue;

  //kdDebug(5800) << "setDtDue says date is " << mDtDue.toString() << endl;

  /*const Alarm::List& alarms = alarms();
  for (Alarm* alarm = alarms.first(); alarm; alarm = alarms.next())
    alarm->setAlarmStart(mDtDue);*/

  updated();
}

QDateTime Todo::dtDue() const
{
  return mDtDue;
}

QString Todo::dtDueTimeStr() const
{
  return KGlobal::locale()->formatTime(mDtDue.time());
}

QString Todo::dtDueDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(mDtDue.date(),shortfmt);
}

QString Todo::dtDueStr() const
{
  return KGlobal::locale()->formatDateTime(mDtDue);
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
  mHasStartDate = f;
  updated();
}

#if 0
void Todo::setStatus(const QString &statStr)
{
  if (mReadOnly) return;
  QString ss(statStr.upper());

  if (ss == "X-ACTION")
    mStatus = NEEDS_ACTION;
  else if (ss == "NEEDS ACTION")
    mStatus = NEEDS_ACTION;
  else if (ss == "ACCEPTED")
    mStatus = ACCEPTED;
  else if (ss == "SENT")
    mStatus = SENT;
  else if (ss == "TENTATIVE")
    mStatus = TENTATIVE;
  else if (ss == "CONFIRMED")
    mStatus = CONFIRMED;
  else if (ss == "DECLINED")
    mStatus = DECLINED;
  else if (ss == "COMPLETED")
    mStatus = COMPLETED;
  else if (ss == "DELEGATED")
    mStatus = DELEGATED;
  else
    kdDebug(5800) << "error setting status, unknown status!" << endl;

  updated();
}

void Todo::setStatus(int status)
{
  if (mReadOnly) return;
  mStatus = status;
  updated();
}

int Todo::status() const
{
  return mStatus;
}

QString Todo::statusStr() const
{
  switch(mStatus) {
  case NEEDS_ACTION:
    return QString("NEEDS ACTION");
    break;
  case ACCEPTED:
    return QString("ACCEPTED");
    break;
  case SENT:
    return QString("SENT");
    break;
  case TENTATIVE:
    return QString("TENTATIVE");
    break;
  case CONFIRMED:
    return QString("CONFIRMED");
    break;
  case DECLINED:
    return QString("DECLINED");
    break;
  case COMPLETED:
    return QString("COMPLETED");
    break;
  case DELEGATED:
    return QString("DELEGATED");
    break;
  }
  return QString("");
}
#endif

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
  mHasCompletedDate = true;
  mPercentComplete = 100;
  mCompleted = completed;
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

