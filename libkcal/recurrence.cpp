/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
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

// $Id$	

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "incidence.h"

#include "recurrence.h"

using namespace KCal;

Recurrence::Recurrence(Incidence *parent)
{
  mParent = parent;

  recurs = rNone; // by default, it's not a recurring event.
  //  rDays.resize(7); // can't initialize in the header
  rMonthDays.setAutoDelete(TRUE);
  rMonthPositions.setAutoDelete(TRUE);
  rYearNums.setAutoDelete(TRUE);

  mRecurReadOnly = false;
  mRecurExDatesCount = 0;
}

Recurrence::~Recurrence()
{
}

ushort Recurrence::doesRecur() const
{
  return recurs;
}

bool Recurrence::recursOnPure(const QDate &qd) const
{
  // first off, check to see if the flag is even set
  if (recurs == rNone)
    return FALSE;

  // it recurs, let's check to see if this date is valid
  switch(recurs) {
  case rDaily:
    return recursDaily(qd);
    break;
  case rWeekly:
    return recursWeekly(qd);
    break;
  case rMonthlyPos:
    return recursMonthlyByPos(qd);
    break;
  case rMonthlyDay:
    return recursMonthlyByDay(qd);
    break;
  case rYearlyMonth:
    return recursYearlyByMonth(qd);
    break;
  case rYearlyDay:
    return recursYearlyByDay(qd);
    break;
  default:
    // catch-all.  Should never get here.
    kdDebug(5800) << "Control should never reach here in recursOn()!" << endl;
    return FALSE;
    break;
  } // case
}

bool Recurrence::recursDaily(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int i;

  if ((qd >= dStart) && 
      (((qd <= dStart.addDays((rDuration-1+mRecurExDatesCount)*rFreq)) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    i = dStart.daysTo(qd);
    // here's the real check...
    if ((i % rFreq) == 0) {
      return TRUE;
    }
    else // frequency didn't match
      return FALSE;
  } 
  // the date queried fell outside the range of the event
  return FALSE;
}

bool Recurrence::recursWeekly(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int i;

  
  i = ((rDuration-1+mRecurExDatesCount)*7) + (7 - dStart.dayOfWeek());
  if ((qd >= dStart) &&
      (((qd <= dStart.addDays(i*rFreq)) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check.
    i = dStart.daysTo(qd)/7;
    if ((i % rFreq) == 0) {
      // check if the bits set match today.
      i = qd.dayOfWeek()-1;
      if (rDays.testBit((uint) i))
	return TRUE;
      else 
	return FALSE;
    } else // frequency didn't match
      return FALSE;
  }
  // the date queried fell outside the range of the event
  return FALSE;
}

bool Recurrence::recursMonthlyByDay(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int monthsAhead = 0;
  int i = 0;
  QPtrListIterator<int> qlid(rMonthDays);
  // calculate how many months ahead this date is from the original
  // event's date
  
  // calculate year's months first
  monthsAhead = (qd.year() - dStart.year()) * 12;
  
  // calculate month offset within the year.
  i = qd.month() - dStart.month(); // may be positive or negative
  
  monthsAhead += i; // add the month offset in
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((monthsAhead <= (rDuration-1+mRecurExDatesCount)*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((monthsAhead % rFreq) == 0) {
      i = qd.day();
      for (; qlid.current(); ++qlid) {
	if (*qlid.current() < 0) {
	  if (i == (qd.daysInMonth()-*qlid.current()+1))
	    return TRUE;
	} else { 
	  if (i == *qlid.current())
	    return TRUE;
	}
      } // for loop
      // no dates matched, return false
      return FALSE;
    } else // frequency didn't match
      return FALSE;
  } 
  // outsize proper date range
  return FALSE;
}

bool Recurrence::recursMonthlyByPos(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int monthsAhead = 0;
  int i = 0;
  QPtrListIterator<rMonthPos> qlip(rMonthPositions);

  // calculate how many months ahead this date is from the original
  // event's date
  
  // calculate year's months first
  monthsAhead = (qd.year() - dStart.year()) * 12;
  
  // calculate month offset within the year.
  i = qd.month() - dStart.month(); // may be positive or negative
  
  monthsAhead += i; // add the month offset in
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((monthsAhead <= (rDuration-1+mRecurExDatesCount)*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((monthsAhead % rFreq) == 0) {
      i = weekOfMonth(qd);
      // check to see if this day of the week isn't found in the first
      // week of the month.
      QDate first(qd.year(), qd.month(), 1);
      if (qd.dayOfWeek() < first.dayOfWeek())
	--i;

      // now loop through the list of modifiers, and check them
      // all against the day of the month
      for (; qlip.current(); ++qlip) {
	if (qlip.current()->negative) {
	  i = 5 - i; // convert to negative offset format
	}
	// check position offset
	if (i == qlip.current()->rPos) {
	  // check day(s)
	  if (qlip.current()->rDays.testBit((uint) qd.dayOfWeek()-1))
	    return TRUE;
	} // if position test
      } // for loop 
      // no dates matched as true, must be false.
      return FALSE;
    } else // frequency didn't match
      return FALSE;
  }
  // the date queried fell outside the range of the event
  return FALSE;
}

bool Recurrence::recursYearlyByMonth(const QDate &qd) const 
{
  QDate dStart = mRecurStart.date();
  int yearsAhead = 0;
  int  i = 0;
  QPtrListIterator<int> qlin(rYearNums);

  // calculate how many years ahead this date is from the original
  // event's date
  
  yearsAhead = (qd.year() - dStart.year());
  
  // check to see if the date is in the proper range
  if ((qd >= dStart) &&
      (((yearsAhead <= (rDuration-1+mRecurExDatesCount)*rFreq) && (rDuration > 0)) || 
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((yearsAhead % rFreq) == 0) {
      i = qd.month();
      for (; qlin.current(); ++qlin) {
	if (i == *qlin.current())
	  if (qd.day() == dStart.day())
	    return TRUE;
      }
      // no dates matched, return false
      return FALSE;
    } else
      // frequency didn't match
      return FALSE;
  } // outside proper date range
  return FALSE;
}

bool Recurrence::recursYearlyByDay(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int yearsAhead = 0;
  int i = 0;
  QPtrListIterator<int> qlin(rYearNums);

  // calculate how many years ahead this date is from the original
  // event's date
  
  yearsAhead = (qd.year() - dStart.year());
  
  // check to see if date is in the proper range
  if ((qd >= dStart) &&
      (((yearsAhead <= (rDuration-1+mRecurExDatesCount)*rFreq) && (rDuration > 0)) ||
       (rDuration == -1) ||
       ((rDuration == 0) && (qd <= rEndDate)))) {
    // do frequency check
    if ((yearsAhead % rFreq) == 0) {
      i = qd.dayOfYear();
      // correct for leapYears
      if (!QDate::leapYear(dStart.year()) &&
	  QDate::leapYear(qd.year()) &&
	  qd > QDate(qd.year(), 2, 28))
	--i;
      if (QDate::leapYear(dStart.year()) &&
	  !QDate::leapYear(qd.year()) &&
	  qd > QDate(qd.year(), 2, 28))
	++i;

      for (; qlin.current(); ++qlin) {
	if (i == *qlin.current())
	  return TRUE;
      }
      // no dates matched, return false
      return FALSE;
    } else 
      // frequency didn't match
      return FALSE;
  } // outside allowable date range
  return FALSE;
}

void Recurrence::unsetRecurs()
{
  if (mRecurReadOnly) return;
  recurs = rNone;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
}

void Recurrence::setDaily(int _rFreq, int _rDuration)
{
  if (mRecurReadOnly) return;
  recurs = rDaily;

  rFreq = _rFreq;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  mParent->updated();
}

void Recurrence::setDaily(int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly) return;
  recurs = rDaily;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  mParent->updated();
}

int Recurrence::frequency() const
{
  return rFreq;
}

int Recurrence::duration() const
{
  return rDuration;
}

const QDate &Recurrence::endDate() const
{
  return rEndDate;
}

QString Recurrence::endDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(rEndDate,shortfmt);
}

const QBitArray &Recurrence::days() const
{
  return rDays;
}

const QPtrList<Recurrence::rMonthPos> &Recurrence::monthPositions() const
{
  return rMonthPositions;
}

const QPtrList<int> &Recurrence::monthDays() const
{
  return rMonthDays;
}

void Recurrence::setWeekly(int _rFreq, const QBitArray &_rDays, 
			       int _rDuration)
{
  if (mRecurReadOnly) return;
  recurs = rWeekly;

  rFreq = _rFreq;
  rDays = _rDays;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  mParent->updated();
}

void Recurrence::setWeekly(int _rFreq, const QBitArray &_rDays, 
			       const QDate &_rEndDate)
{
  if (mRecurReadOnly) return;
  recurs = rWeekly;

  rFreq = _rFreq;
  rDays = _rDays;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  mParent->updated();
}

void Recurrence::setMonthly(short type, int _rFreq, int _rDuration)
{
  if (mRecurReadOnly) return;
  recurs = type;

  rFreq = _rFreq;
  rDuration = _rDuration;
  rYearNums.clear();
  mParent->updated();
}

void Recurrence::setMonthly(short type, int _rFreq, 
				const QDate &_rEndDate)
{
  if (mRecurReadOnly) return;
  recurs = type;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0; // set to 0 because there is an end date
  rYearNums.clear();
  mParent->updated();
}

void Recurrence::addMonthlyPos(short _rPos, const QBitArray &_rDays)
{
  if (mRecurReadOnly) return;
  rMonthPos *tmpPos = new rMonthPos;
  tmpPos->negative = FALSE;
  if (_rPos < 0) {
    _rPos = 0 - _rPos; // take abs()
    tmpPos->negative = TRUE;
  }
  tmpPos->rPos = _rPos;
  tmpPos->rDays = _rDays;
  rMonthPositions.append(tmpPos);
  mParent->updated();
}

void Recurrence::addMonthlyDay(short _rDay)
{
  if (mRecurReadOnly) return;
  int *tmpDay = new int;
  *tmpDay = _rDay;
  rMonthDays.append(tmpDay);
  mParent->updated();
}

void Recurrence::setYearly(int type, int _rFreq, int _rDuration)
{
  if (mRecurReadOnly) return;
  recurs = type;
  
  rFreq = _rFreq;
  rDuration = _rDuration;
  rMonthPositions.clear();
  rMonthDays.clear();
  mParent->updated();
}

void Recurrence::setYearly(int type, int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly) return;
  recurs = type;

  rFreq = _rFreq;
  rEndDate = _rEndDate;
  rDuration = 0;
  rMonthPositions.clear();
  rMonthDays.clear();
  mParent->updated();
}

const QPtrList<int> &Recurrence::yearNums() const
{
  return rYearNums;
}

void Recurrence::addYearlyNum(short _rNum)
{
  if (mRecurReadOnly) return;

  int *tmpNum = new int;
  *tmpNum = _rNum;
  rYearNums.append(tmpNum);
  
  mParent->updated();
}

/***************************** PROTECTED FUNCTIONS ***************************/

// this should return the week of the month for the date
int Recurrence::weekOfMonth(const QDate &qd) const
{
  QDate firstDate(qd.year(), qd.month(), 1);
  // I don't really know what formulas I'm using here.  :)
  int firstWeekNum(1 +(firstDate.dayOfYear() - firstDate.dayOfWeek() + 6)/7);
  int thisWeekNum(1 +(qd.dayOfYear() - qd.dayOfWeek() + 6)/7);
  return (thisWeekNum - firstWeekNum + 1);
}
