/*
    This file is part of libkcal.
    Copyright (c) 1998 Preston Brown
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>

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

#include <limits.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include "incidence.h"

#include "recurrence.h"

using namespace KCal;

const QDate Recurrence::MAX_DATE(3000, 1, 1);   // Maximum date = 1-Jan-3000
Recurrence::Feb29Type Recurrence::mFeb29YearlyDefaultType = Recurrence::rMar1;


Recurrence::Recurrence(Incidence *parent, int compatVersion)
: recurs(rNone),   // by default, it's not a recurring event
  rWeekStart(1),   // default is Monday
  rDays(7),
  mUseCachedEndDT(false),
  mFloats(parent ? parent->doesFloat() : false),
  mRecurReadOnly(false),
  mFeb29YearlyType(mFeb29YearlyDefaultType),
  mCompatVersion(compatVersion ? compatVersion : INT_MAX),
  mCompatRecurs(rNone),
  mCompatDuration(0),
  mParent(parent)
{
  rMonthDays.setAutoDelete( true );
  rMonthPositions.setAutoDelete( true );
  rYearNums.setAutoDelete( true );
}

Recurrence::Recurrence(const Recurrence &r, Incidence *parent)
: recurs(r.recurs),
  rWeekStart(r.rWeekStart),
  rDays(r.rDays.copy()),
  rFreq(r.rFreq),
  rDuration(r.rDuration),
  rEndDateTime(r.rEndDateTime),
  mCachedEndDT(r.mCachedEndDT),
  mUseCachedEndDT(r.mUseCachedEndDT),
  mRecurStart(r.mRecurStart),
  mFloats(r.mFloats),
  mRecurReadOnly(r.mRecurReadOnly),
  mFeb29YearlyType(r.mFeb29YearlyType),
  mCompatVersion(r.mCompatVersion),
  mCompatRecurs(r.mCompatRecurs),
  mCompatDuration(r.mCompatDuration),
  mParent(parent)
{
  for (QPtrListIterator<rMonthPos> mp(r.rMonthPositions);  mp.current();  ++mp) {
    rMonthPos *tmp = new rMonthPos;
    tmp->rPos     = mp.current()->rPos;
    tmp->negative = mp.current()->negative;
    tmp->rDays    = mp.current()->rDays.copy();
    rMonthPositions.append(tmp);
  }
  for (QPtrListIterator<int> md(r.rMonthDays);  md.current();  ++md) {
    int *tmp = new int;
    *tmp = *md.current();
    rMonthDays.append(tmp);
  }
  for (QPtrListIterator<int> yn(r.rYearNums);  yn.current();  ++yn) {
    int *tmp = new int;
    *tmp = *yn.current();
    rYearNums.append(tmp);
  }
  rMonthDays.setAutoDelete( true );
  rMonthPositions.setAutoDelete( true );
  rYearNums.setAutoDelete( true );
}

Recurrence::~Recurrence()
{
}


bool Recurrence::operator==( const Recurrence& r2 ) const
{
  if ( recurs == rNone  &&  r2.recurs == rNone )
    return true;
  if ( recurs != r2.recurs
  ||   rFreq != r2.rFreq
  ||   rDuration != r2.rDuration
  ||   ( !rDuration && rEndDateTime != r2.rEndDateTime )
  ||   mRecurStart != r2.mRecurStart
  ||   mFloats != r2.mFloats
  ||   mRecurReadOnly != r2.mRecurReadOnly )
    return false;
  // no need to compare mCompat* and mParent
  // OK to compare the pointers
  switch ( recurs )
  {
    case rWeekly:
      return rDays == r2.rDays
      &&     rWeekStart == r2.rWeekStart;
    case rMonthlyPos:
      return rMonthPositions == r2.rMonthPositions;
    case rMonthlyDay:
      return rMonthDays == r2.rMonthDays;
    case rYearlyPos:
      return rYearNums == r2.rYearNums
      &&     rMonthPositions == r2.rMonthPositions;
    case rYearlyMonth:
      return rYearNums == r2.rYearNums
      &&     rMonthDays == r2.rMonthDays
      &&     mFeb29YearlyType == r2.mFeb29YearlyType;
    case rYearlyDay:
      return rYearNums == r2.rYearNums;
    case rNone:
    case rMinutely:
    case rHourly:
    case rDaily:
    default:
      return true;
  }
}


void Recurrence::setCompatVersion(int version)
{
  mCompatVersion = version ? version : INT_MAX;
  mUseCachedEndDT = false;
}

ushort Recurrence::doesRecur() const
{
  return recurs;
}

bool Recurrence::recursOnPure(const QDate &qd) const
{
  switch(recurs) {
    case rMinutely:
      return recursSecondly(qd, rFreq*60);
    case rHourly:
      return recursSecondly(qd, rFreq*3600);
    case rDaily:
      return recursDaily(qd);
    case rWeekly:
      return recursWeekly(qd);
    case rMonthlyPos:
    case rMonthlyDay:
      return recursMonthly(qd);
    case rYearlyMonth:
      return recursYearlyByMonth(qd);
    case rYearlyDay:
      return recursYearlyByDay(qd);
    case rYearlyPos:
      return recursYearlyByPos(qd);
    default:
      // catch-all.  Should never get here.
      kdError(5800) << "Control should never reach here in recursOnPure()!" << endl;
    case rNone:
      return false;
  } // case
}

bool Recurrence::recursAtPure(const QDateTime &dt) const
{
  switch(recurs) {
  case rMinutely:
    return recursMinutelyAt(dt, rFreq);
  case rHourly:
    return recursMinutelyAt(dt, rFreq*60);
  default:
    if (dt.time() != mRecurStart.time())
      return false;
    switch(recurs) {
      case rDaily:
        return recursDaily(dt.date());
      case rWeekly:
        return recursWeekly(dt.date());
      case rMonthlyPos:
      case rMonthlyDay:
        return recursMonthly(dt.date());
      case rYearlyMonth:
        return recursYearlyByMonth(dt.date());
      case rYearlyDay:
        return recursYearlyByDay(dt.date());
      case rYearlyPos:
        return recursYearlyByPos(dt.date());
      default:
        // catch-all.  Should never get here.
        kdError(5800) << "Control should never reach here in recursAtPure()!" << endl;
      case rNone:
        return false;
    }
  } // case
}

QDate Recurrence::endDate(bool *result) const
{
  return endDateTime(result).date();
}

QDateTime Recurrence::endDateTime(bool *result) const
{
  int count = 0;
  if (result)
    *result = true;
  QDate end;
  if (recurs != rNone) {
    if (rDuration < 0)
      return QDateTime();    // infinite recurrence
    if (rDuration == 0)
      return rEndDateTime;

    // The end date is determined by the recurrence count
    if (mUseCachedEndDT) {
      if (result && !mCachedEndDT.isValid())
        *result = false;     // error - there is no recurrence
      return mCachedEndDT;   // avoid potentially long calculation
    }

    mUseCachedEndDT = true;
    switch (recurs)
    {
    case rMinutely:
      mCachedEndDT = mRecurStart.addSecs((rDuration-1)*rFreq*60);
      return mCachedEndDT;
    case rHourly:
      mCachedEndDT = mRecurStart.addSecs((rDuration-1)*rFreq*3600);
      return mCachedEndDT;
    case rDaily:
      mCachedEndDT = mRecurStart.addDays((rDuration-1)*rFreq);
      return mCachedEndDT;

    case rWeekly:
      count = weeklyCalc(END_DATE_AND_COUNT, end);
      break;
    case rMonthlyPos:
    case rMonthlyDay:
      count = monthlyCalc(END_DATE_AND_COUNT, end);
      break;
    case rYearlyMonth:
      count = yearlyMonthCalc(END_DATE_AND_COUNT, end);
      break;
    case rYearlyDay:
      count = yearlyDayCalc(END_DATE_AND_COUNT, end);
      break;
    case rYearlyPos:
      count = yearlyPosCalc(END_DATE_AND_COUNT, end);
      break;
    default:
      // catch-all.  Should never get here.
      kdError(5800) << "Control should never reach here in endDate()!" << endl;
      mUseCachedEndDT = false;
      break;
    }
  }
  if (!count) {
    if (result)
      *result = false;
    mCachedEndDT = QDateTime();   // error - there is no recurrence
  }
  else
    mCachedEndDT = QDateTime(end, mRecurStart.time());
  return mCachedEndDT;
}

QString Recurrence::endDateStr(bool shortfmt) const
{
  return KGlobal::locale()->formatDate(rEndDateTime.date(),shortfmt);
}

void Recurrence::setEndDate(const QDate &date)
{
  setEndDateTime(QDateTime(date, mRecurStart.time()));
}

void Recurrence::setEndDateTime(const QDateTime &dateTime)
{
  if (mRecurReadOnly) return;
  rEndDateTime = dateTime;
  rDuration = 0; // set to 0 because there is an end date/time
  mCompatDuration = 0;
  mUseCachedEndDT = false;
}

int Recurrence::duration() const
{
  return rDuration;
}

int Recurrence::durationTo(const QDate &date) const
{
  QDate d = date;
  return recurCalc(COUNT_TO_DATE, d);
}

int Recurrence::durationTo(const QDateTime &datetime) const
{
  QDateTime dt = datetime;
  return recurCalc(COUNT_TO_DATE, dt);
}

void Recurrence::setDuration(int _rDuration)
{
  if (mRecurReadOnly) return;
  if (_rDuration > 0) {
    rDuration = _rDuration;
    // Compatibility mode is only needed when reading the calendar in ICalFormatImpl,
    // so explicitly setting the duration means no backwards compatibility is needed.
    mCompatDuration = 0;
  }
  mUseCachedEndDT = false;
}

void Recurrence::unsetRecurs()
{
  if (mRecurReadOnly) return;
  recurs = rNone;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  mUseCachedEndDT = false;
}

void Recurrence::setRecurStart(const QDateTime &start)
{
  mRecurStart = start;
  mFloats = false;
  switch (recurs)
  {
    case rMinutely:
    case rHourly:
      break;
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
    default:
      rEndDateTime.setTime(start.time());
      break;
  }
  mUseCachedEndDT = false;
}

void Recurrence::setRecurStart(const QDate &start)
{
  mRecurStart.setDate(start);
  mRecurStart.setTime(QTime(0,0,0));
  switch (recurs)
  {
    case rMinutely:
    case rHourly:
      break;
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
    default:
      mFloats = true;
      break;
  }
  mUseCachedEndDT = false;
}

void Recurrence::setFloats(bool f)
{
  if (f && mFloats  ||  !f && !mFloats)
    return;    // no change

  switch (recurs)
  {
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
      break;
    case rMinutely:
    case rHourly:
    default:
      return;     // can't set sub-daily to floating
  }
  mFloats = f;
  if (f) {
    mRecurStart.setTime(QTime(0,0,0));
    rEndDateTime.setTime(QTime(0,0,0));
  }
  mUseCachedEndDT = false;
}

int Recurrence::frequency() const
{
  return rFreq;
}

void Recurrence::setFrequency(int freq)
{
  if (mRecurReadOnly || freq <= 0) return;
  rFreq = freq;
  mUseCachedEndDT = false;
}

const QBitArray &Recurrence::days() const
{
  return rDays;
}

const QPtrList<Recurrence::rMonthPos> &Recurrence::monthPositions() const
{
  return rMonthPositions;
}

const QPtrList<Recurrence::rMonthPos> &Recurrence::yearMonthPositions() const
{
  return rMonthPositions;
}

const QPtrList<int> &Recurrence::monthDays() const
{
  return rMonthDays;
}

void Recurrence::setMinutely(int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  setDailySub(rMinutely, _rFreq, _rDuration);
}

void Recurrence::setMinutely(int _rFreq, const QDateTime &_rEndDateTime)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  rEndDateTime = _rEndDateTime;
  setDailySub(rMinutely, _rFreq, 0);
}

void Recurrence::setHourly(int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  setDailySub(rHourly, _rFreq, _rDuration);
}

void Recurrence::setHourly(int _rFreq, const QDateTime &_rEndDateTime)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  rEndDateTime = _rEndDateTime;
  setDailySub(rHourly, _rFreq, 0);
}

void Recurrence::setDaily(int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  setDailySub(rDaily, _rFreq, _rDuration);
}

void Recurrence::setDaily(int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  rEndDateTime.setDate(_rEndDate);
  rEndDateTime.setTime(mRecurStart.time());
  setDailySub(rDaily, _rFreq, 0);
}

void Recurrence::setWeekly(int _rFreq, const QBitArray &_rDays,
                           int _rDuration, int _rWeekStart)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  mUseCachedEndDT = false;

  recurs = rWeekly;
  rFreq = _rFreq;
  rDays = _rDays;
  rWeekStart = _rWeekStart;
  rDuration = _rDuration;
  if (mCompatVersion < 310 && _rDuration > 0) {
    // Backwards compatibility for KDE < 3.1.
    // rDuration was set to the number of time periods to recur,
    // with week start always on a Monday.
    // Convert this to the number of occurrences.
    mCompatDuration = _rDuration;
    int weeks = ((mCompatDuration-1)*7) + (7 - mRecurStart.date().dayOfWeek());
    QDate end(mRecurStart.date().addDays(weeks * rFreq));
    rDuration = INT_MAX;    // ensure that weeklyCalc() does its job correctly
    rDuration = weeklyCalc(COUNT_TO_DATE, end);
  } else {
    mCompatDuration = 0;
  }
  rMonthPositions.clear();
  rMonthDays.clear();
  if (mParent) mParent->updated();
}

void Recurrence::setWeekly(int _rFreq, const QBitArray &_rDays,
                           const QDate &_rEndDate, int _rWeekStart)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  mUseCachedEndDT = false;

  recurs = rWeekly;
  rFreq = _rFreq;
  rDays = _rDays;
  rWeekStart = _rWeekStart;
  rEndDateTime.setDate(_rEndDate);
  rEndDateTime.setTime(mRecurStart.time());
  rDuration = 0; // set to 0 because there is an end date
  mCompatDuration = 0;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  if (mParent) mParent->updated();
}

void Recurrence::setMonthly(short type, int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  mUseCachedEndDT = false;

  recurs = type;
  rFreq = _rFreq;
  rDuration = _rDuration;
  if (mCompatVersion < 310)
    mCompatDuration = (_rDuration > 0) ? _rDuration : 0;
  rYearNums.clear();
  if (mParent) mParent->updated();
}

void Recurrence::setMonthly(short type, int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  mUseCachedEndDT = false;

  recurs = type;
  rFreq = _rFreq;
  rEndDateTime.setDate(_rEndDate);
  rEndDateTime.setTime(mRecurStart.time());
  rDuration = 0; // set to 0 because there is an end date
  mCompatDuration = 0;
  rYearNums.clear();
  if (mParent) mParent->updated();
}

void Recurrence::addMonthlyPos(short _rPos, const QBitArray &_rDays)
{
  if ( recurs == rMonthlyPos || recurs == rYearlyPos )
    addMonthlyPos_(_rPos, _rDays);
}

void Recurrence::addMonthlyPos_(short _rPos, const QBitArray &_rDays)
{
  if (mRecurReadOnly
  ||  _rPos == 0 || _rPos > 5 || _rPos < -5)    // invalid week number
    return;

  mUseCachedEndDT = false;
  for (rMonthPos* it = rMonthPositions.first();  it;  it = rMonthPositions.next()) {
    int itPos = it->negative ? -it->rPos : it->rPos;
    if (_rPos == itPos) {
      // This week is already in the list.
      // Combine the specified days with those in the list.
      it->rDays |= _rDays;
      if (mParent) mParent->updated();
      return;
    }
  }
  // Add the new position to the list
  rMonthPos *tmpPos = new rMonthPos;
  if (_rPos > 0) {
    tmpPos->rPos = _rPos;
    tmpPos->negative = false;
  } else {
    tmpPos->rPos = -_rPos; // take abs()
    tmpPos->negative = true;
  }
  tmpPos->rDays = _rDays;
  tmpPos->rDays.detach();
  rMonthPositions.append(tmpPos);

  if (mCompatVersion < 310 && mCompatDuration > 0) {
    // Backwards compatibility for KDE < 3.1.
    // rDuration was set to the number of time periods to recur.
    // Convert this to the number of occurrences.
    int monthsAhead = (mCompatDuration-1) * rFreq;
    int month = mRecurStart.date().month() - 1 + monthsAhead;
    QDate end(mRecurStart.date().year() + month/12, month%12 + 1, 31);
    rDuration = INT_MAX;    // ensure that recurCalc() does its job correctly
    rDuration = recurCalc(COUNT_TO_DATE, end);
  }

  if (mParent) mParent->updated();
}

void Recurrence::addMonthlyDay(short _rDay)
{
  if (mRecurReadOnly || (recurs != rMonthlyDay && recurs != rYearlyMonth)
  ||  _rDay == 0 || _rDay > 31 || _rDay < -31)   // invalid day number
    return;
  for (int* it = rMonthDays.first();  it;  it = rMonthDays.next()) {
    if (_rDay == *it)
      return;        // this day is already in the list - avoid duplication
  }
  mUseCachedEndDT = false;

  int *tmpDay = new int;
  *tmpDay = _rDay;
  rMonthDays.append(tmpDay);

  if (mCompatVersion < 310 && mCompatDuration > 0) {
    // Backwards compatibility for KDE < 3.1.
    // rDuration was set to the number of time periods to recur.
    // Convert this to the number of occurrences.
    int monthsAhead = (mCompatDuration-1) * rFreq;
    int month = mRecurStart.date().month() - 1 + monthsAhead;
    QDate end(mRecurStart.date().year() + month/12, month%12 + 1, 31);
    rDuration = INT_MAX;    // ensure that recurCalc() does its job correctly
    rDuration = recurCalc(COUNT_TO_DATE, end);
  }

  if (mParent) mParent->updated();
}

void Recurrence::setYearly(int type, int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  if (mCompatVersion < 310)
    mCompatDuration = (_rDuration > 0) ? _rDuration : 0;
  setYearly_(type, mFeb29YearlyDefaultType, _rFreq, _rDuration);
}

void Recurrence::setYearly(int type, int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  rEndDateTime.setDate(_rEndDate);
  rEndDateTime.setTime(mRecurStart.time());
  mCompatDuration = 0;
  setYearly_(type, mFeb29YearlyDefaultType, _rFreq, 0);
}

void Recurrence::setYearlyByDate(Feb29Type type, int _rFreq, int _rDuration)
{
  setYearlyByDate(0, type, _rFreq, _rDuration);
}

void Recurrence::setYearlyByDate(Feb29Type type, int _rFreq, const QDate &_rEndDate)
{
  setYearlyByDate(0, type, _rFreq, _rEndDate);
}

void Recurrence::setYearlyByDate(int day, Feb29Type type, int _rFreq, int _rDuration)
{
  if (mRecurReadOnly || _rFreq <= 0 || _rDuration == 0 || _rDuration < -1)
    return;
  if (mCompatVersion < 310)
    mCompatDuration = (_rDuration > 0) ? _rDuration : 0;
  setYearly_(rYearlyMonth, type, _rFreq, _rDuration);
  if (day)
    addMonthlyDay(day);
}

void Recurrence::setYearlyByDate(int day, Feb29Type type, int _rFreq, const QDate &_rEndDate)
{
  if (mRecurReadOnly || _rFreq <= 0) return;
  rEndDateTime.setDate(_rEndDate);
  rEndDateTime.setTime(mRecurStart.time());
  mCompatDuration = 0;
  setYearly_(rYearlyMonth, type, _rFreq, 0);
  if (day)
    addMonthlyDay(day);
}

void Recurrence::addYearlyMonthPos(short _rPos, const QBitArray &_rDays)
{
  if (recurs == rYearlyPos)
    addMonthlyPos_(_rPos, _rDays);
}

const QPtrList<int> &Recurrence::yearNums() const
{
  return rYearNums;
}

void Recurrence::addYearlyNum(short _rNum)
{
  if (mRecurReadOnly
  ||  (recurs != rYearlyMonth && recurs != rYearlyDay && recurs != rYearlyPos)
  ||  _rNum <= 0)    // invalid day/month number
    return;

  if (mCompatVersion < 310 && mCompatRecurs == rYearlyDay) {
    // Backwards compatibility for KDE < 3.1.
    // Dates were stored as day numbers, with a fiddle to take account of leap years.
    // Convert the day number to a month.
    if (_rNum <= 0 || _rNum > 366 || (_rNum == 366 && mRecurStart.date().daysInYear() < 366))
      return;     // invalid day number
    _rNum = QDate(mRecurStart.date().year(), 1, 1).addDays(_rNum - 1).month();
  } else
  if ((recurs == rYearlyMonth || recurs == rYearlyPos) && _rNum > 12
  ||  recurs == rYearlyDay && _rNum > 366)
    return;     // invalid day number

  uint i = 0;
  for (int* it = rYearNums.first();  it && _rNum >= *it;  it = rYearNums.next()) {
    if (_rNum == *it)
      return;        // this day/month is already in the list - avoid duplication
    ++i;
  }
  mUseCachedEndDT = false;

  int *tmpNum = new int;
  *tmpNum = _rNum;
  rYearNums.insert(i, tmpNum);   // insert the day/month in a sorted position

  if (mCompatVersion < 310 && mCompatDuration > 0) {
    // Backwards compatibility for KDE < 3.1.
    // rDuration was set to the number of time periods to recur.
    // Convert this to the number of occurrences.
    QDate end(mRecurStart.date().year() + (mCompatDuration-1)*rFreq, 12, 31);
    rDuration = INT_MAX;    // ensure that recurCalc() does its job correctly
    rDuration = recurCalc(COUNT_TO_DATE, end);
  }

  if (mParent) mParent->updated();
}


QValueList<QTime> Recurrence::recurTimesOn(const QDate &date) const
{
  QValueList<QTime> times;
  switch (recurs)
  {
    case rMinutely:
    case rHourly:
      if ((date >= mRecurStart.date()) &&
          ((rDuration > 0) && (date <= endDate()) ||
           ((rDuration == 0) && (date <= rEndDateTime.date())) ||
           (rDuration == -1))) {
        // The date queried falls within the range of the event.
        int secondFreq = rFreq * (recurs == rMinutely ? 60 : 3600);
        int after = mRecurStart.secsTo(QDateTime(date)) - 1;
        int count = (after + 24*3600) / secondFreq - after / secondFreq;
        if (count) {
          // It recurs at least once on the given date
          QTime t = mRecurStart.addSecs((after / secondFreq) * secondFreq).time();
          while (--count >= 0) {
            t = t.addSecs(secondFreq);
            times.append(t);
          }
        }
      }
      break;
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
      if (recursOnPure(date))
        times.append(mRecurStart.time());
      break;
    default:
      break;
  }
  return times;
}

QDateTime Recurrence::getNextDateTime(const QDateTime &preDateTime, bool *last) const
{
  int freq;
  switch (recurs)
  {
    case rMinutely:
      freq = rFreq * 60;
      break;
    case rHourly:
      freq = rFreq * 3600;
      break;
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos: {
      QDate preDate = preDateTime.date();
      if (!mFloats && mRecurStart.time() > preDateTime.time())
        preDate = preDate.addDays(-1);
      return QDateTime(getNextDateNoTime(preDate, last), mRecurStart.time());
    }
    default:
      return QDateTime();
  }

  // It's a sub-daily recurrence
  if (last)
    *last = false;
  if (preDateTime < mRecurStart)
    return mRecurStart;
  int count = mRecurStart.secsTo(preDateTime) / freq + 2;
  if (rDuration > 0) {
    if (count > rDuration)
      return QDateTime();
    if (last && count == rDuration)
      *last = true;
  }
  QDateTime endtime = mRecurStart.addSecs((count - 1)*freq);
  if (rDuration == 0) {
    if (endtime > rEndDateTime)
      return QDateTime();
    if (last && endtime == rEndDateTime)
      *last = true;
  }
  return endtime;
}

QDate Recurrence::getNextDate(const QDate &preDate, bool *last) const
{
  switch (recurs)
  {
    case rMinutely:
    case rHourly:
      return getNextDateTime(QDateTime(preDate, QTime(23,59,59)), last).date();
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
      return getNextDateNoTime(preDate, last);
    default:
      return QDate();
  }
}


QDateTime Recurrence::getPreviousDateTime(const QDateTime &afterDateTime, bool *last) const
{
  int freq;
  switch (recurs)
  {
    case rMinutely:
      freq = rFreq * 60;
      break;
    case rHourly:
      freq = rFreq * 3600;
      break;
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos: {
      QDate afterDate = afterDateTime.date();
      if (!mFloats && mRecurStart.time() < afterDateTime.time())
        afterDate = afterDate.addDays(1);
      return QDateTime(getPreviousDateNoTime(afterDate, last), mRecurStart.time());
    }
    default:
      return QDateTime();
  }

  // It's a sub-daily recurrence
  if (last)
    *last = false;
  if (afterDateTime <= mRecurStart)
    return QDateTime();
  int count = (mRecurStart.secsTo(afterDateTime) - 1) / freq + 1;
  if (rDuration > 0) {
    if (count > rDuration)
      count = rDuration;
    if (last && count == rDuration)
      *last = true;
  }
  QDateTime endtime = mRecurStart.addSecs((count - 1)*freq);
  if (rDuration == 0) {
    if (endtime > rEndDateTime)
      endtime = rEndDateTime;
    if (last && endtime == rEndDateTime)
      *last = true;
  }
  return endtime;
}

QDate Recurrence::getPreviousDate(const QDate &afterDate, bool *last) const
{
  switch (recurs)
  {
    case rMinutely:
    case rHourly:
      return getPreviousDateTime(QDateTime(afterDate, QTime(0,0,0)), last).date();
    case rDaily:
    case rWeekly:
    case rMonthlyPos:
    case rMonthlyDay:
    case rYearlyMonth:
    case rYearlyDay:
    case rYearlyPos:
      return getPreviousDateNoTime(afterDate, last);
    default:
      return QDate();
  }
}


/***************************** PROTECTED FUNCTIONS ***************************/

bool Recurrence::recursSecondly(const QDate &qd, int secondFreq) const
{
  if ((qd >= mRecurStart.date()) &&
      ((rDuration > 0) && (qd <= endDate()) ||
       ((rDuration == 0) && (qd <= rEndDateTime.date())) ||
       (rDuration == -1))) {
    // The date queried falls within the range of the event.
    if (secondFreq < 24*3600)
      return true;      // the event recurs at least once each day
    int after = mRecurStart.secsTo(QDateTime(qd)) - 1;
    if (after / secondFreq != (after + 24*3600) / secondFreq)
      return true;
  }
  return false;
}

bool Recurrence::recursMinutelyAt(const QDateTime &dt, int minuteFreq) const
{
  if ((dt >= mRecurStart) &&
      ((rDuration > 0) && (dt <= endDateTime()) ||
       ((rDuration == 0) && (dt <= rEndDateTime)) ||
       (rDuration == -1))) {
    // The time queried falls within the range of the event.
    if (((mRecurStart.secsTo(dt) / 60) % minuteFreq) == 0)
      return true;
  }
  return false;
}

bool Recurrence::recursDaily(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  if ((dStart.daysTo(qd) % rFreq) == 0) {
    // The date is a day which recurs
    if (qd >= dStart
    &&  ((rDuration > 0 && qd <= endDate()) ||
         (rDuration == 0 && qd <= rEndDateTime.date()) ||
         rDuration == -1)) {
      // The date queried falls within the range of the event.
      return true;
    }
  }
  return false;
}

bool Recurrence::recursWeekly(const QDate &qd) const
{
  int i = qd.dayOfWeek()-1;
  bool weekDayMatches = rDays.testBit( (uint) i);
  QDate dStart = mRecurStart.date();
  if ( mParent && mParent->type() == "Todo" && weekDayMatches ) {
    dStart = dStart.addDays( qd.dayOfWeek() - mRecurStart.date().dayOfWeek() );
  }

  if ((dStart.daysTo(qd)/7) % rFreq == 0 && weekDayMatches ) {
    // The date is in a week which recurs
    if (qd >= dStart
    && ((rDuration > 0 && qd <= endDate()) ||
        (rDuration == 0 && qd <= rEndDateTime.date()) ||
        rDuration == -1)) {
      // The date queried falls within the range of the event.
      return true;
    }
  }
  return false;
}

bool Recurrence::recursMonthly(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int year  = qd.year();
  int month = qd.month();
  int day   = qd.day();
  // calculate how many months ahead this date is from the original
  // event's date
  int monthsAhead = (year - dStart.year()) * 12 + (month - dStart.month());
  if ((monthsAhead % rFreq) == 0) {
    // The date is in a month which recurs
    if (qd >= dStart
    &&  ((rDuration > 0 && qd <= endDate()) ||
         (rDuration == 0 && qd <= rEndDateTime.date()) ||
         rDuration == -1)) {
      // The date queried falls within the range of the event.
      QValueList<int> days;
      int daysInMonth = qd.daysInMonth();
      if (recurs == rMonthlyDay)
        getMonthlyDayDays(days, daysInMonth);
      else if (recurs == rMonthlyPos)
        getMonthlyPosDays(days, daysInMonth, QDate(year, month, 1).dayOfWeek());
      for (QValueList<int>::Iterator it = days.begin();  it != days.end();  ++it) {
        if (*it == day)
          return true;
      }
      // no dates matched
    }
  }
  return false;
}

bool Recurrence::recursYearlyByMonth(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int startDay = dStart.day();
  if (rMonthDays.count())
    startDay = *rMonthDays.getFirst();
  int qday     = qd.day();
  int qmonth   = qd.month();
  int qyear    = qd.year();
  bool match = (qday == startDay);
  if (startDay < 0)
    match = (qday == qd.daysInMonth() + startDay + 1);
  if (!match && startDay == 29 && dStart.month() == 2) {
    // It's a recurrence on February 29th
    switch (mFeb29YearlyType) {
      case rFeb28:
        if (qday == 28 && qmonth == 2 && !QDate::leapYear(qyear))
          match = true;
        break;
      case rMar1:
        if (qday == 1 && qmonth == 3 && !QDate::leapYear(qyear)) {
          qmonth = 2;
          match = true;
        }
        break;
      case rFeb29:
        break;
    }
  }

  if (match) {
    // The day of the month matches. Calculate how many years ahead
    // this date is from the original event's date.
    int yearsAhead = (qyear - dStart.year());
    if (yearsAhead % rFreq == 0) {
      // The date is in a year which recurs
      if (qd >= dStart
      &&  ((rDuration > 0 && qd <= endDate()) ||
           (rDuration == 0 && qd <= rEndDateTime.date()) ||
           rDuration == -1)) {
        // The date queried falls within the range of the event.
        int i = qmonth;
        for (QPtrListIterator<int> qlin(rYearNums); qlin.current(); ++qlin) {
          if (i == *qlin.current())
            return true;
        }
      }
    }
  }
  return false;
}

bool Recurrence::recursYearlyByPos(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  int year  = qd.year();
  int month = qd.month();
  int day   = qd.day();
  // calculate how many years ahead this date is from the original
  // event's date
  int yearsAhead = (year - dStart.year());
  if (yearsAhead % rFreq == 0) {
    // The date is in a year which recurs
    if (qd >= dStart
    &&  ((rDuration > 0 && qd <= endDate()) ||
         (rDuration == 0 && qd <= rEndDateTime.date()) ||
         rDuration == -1)) {
      // The date queried falls within the range of the event.
      for (QPtrListIterator<int> qlin(rYearNums); qlin.current(); ++qlin) {
        if (month == *qlin.current()) {
          // The month recurs
          QValueList<int> days;
          getMonthlyPosDays(days, qd.daysInMonth(), QDate(year, month, 1).dayOfWeek());
          for (QValueList<int>::Iterator it = days.begin();  it != days.end();  ++it) {
            if (*it == day)
              return true;
          }
        }
      }
    }
  }
  return false;
}

bool Recurrence::recursYearlyByDay(const QDate &qd) const
{
  QDate dStart = mRecurStart.date();
  // calculate how many years ahead this date is from the original
  // event's date
  int yearsAhead = (qd.year() - dStart.year());
  if (yearsAhead % rFreq == 0) {
    // The date is in a year which recurs
    if (qd >= dStart
    &&  ((rDuration > 0 && qd <= endDate()) ||
         (rDuration == 0 && qd <= rEndDateTime.date()) ||
         rDuration == -1)) {
      // The date queried falls within the range of the event.
      int i = qd.dayOfYear();
      for (QPtrListIterator<int> qlin(rYearNums); qlin.current(); ++qlin) {
        if (i == *qlin.current())
          return true;
      }
    }
  }
  return false;
}

/* Get the date of the next recurrence, after the specified date.
 * If 'last' is non-null, '*last' is set to true if the next recurrence is the
 * last recurrence, else false.
 * Reply = date of next recurrence, or invalid date if none.
 */
QDate Recurrence::getNextDateNoTime(const QDate &preDate, bool *last) const
{
  if (last)
    *last = false;
  QDate dStart = mRecurStart.date();
  if (preDate < dStart)
    return dStart;
  QDate earliestDate = preDate.addDays(1);
  QDate nextDate;

  switch (recurs) {
    case rDaily:
      nextDate = dStart.addDays((dStart.daysTo(preDate)/rFreq + 1) * rFreq);
      break;

    case rWeekly: {
      QDate start = dStart.addDays(-((dStart.dayOfWeek() - rWeekStart + 7)%7));  // start of week for dStart
      int earliestDayOfWeek = earliestDate.dayOfWeek();
      int weeksAhead = start.daysTo(earliestDate) / 7;
      int notThisWeek = weeksAhead % rFreq;    // zero if this week is a recurring week
      weeksAhead -= notThisWeek;               // latest week which recurred
      int weekday = 0;
      // First check for any remaining day this week, if this week is a recurring week
      if (!notThisWeek)
        weekday = getFirstDayInWeek(earliestDayOfWeek);
      // Check for a day in the next scheduled week
      if (!weekday)
        weekday = getFirstDayInWeek(rWeekStart) + rFreq*7;
      if (weekday)
        nextDate = start.addDays(weeksAhead*7 + weekday - 1);
      break;
    }
    case rMonthlyDay:
    case rMonthlyPos: {
      int startYear  = dStart.year();
      int startMonth = dStart.month();     // 1..12
      int earliestYear = earliestDate.year();
      int monthsAhead = (earliestYear - startYear)*12 + earliestDate.month() - startMonth;
      int notThisMonth = monthsAhead % rFreq;    // zero if this month is a recurring month
      monthsAhead -= notThisMonth;               // latest month which recurred
      // Check for the first later day in the current month
      if (!notThisMonth)
        nextDate = getFirstDateInMonth(earliestDate);
      if (!nextDate.isValid()) {
        /* Check for a day in the next scheduled month.
         * The next check may fail if, for example, it's the 31st day of the month
         * or the 5th Monday, and the month being checked is February or a 30-day month,
         * so limit the number of iterations.
         */
		QDate end = (rDuration >= 0) ? endDate() : MAX_DATE;
        int maxMonthsAhead = (end.year() - startYear)*12 + end.month() - startMonth;
		monthsAhead += rFreq;
        int maxIter = maxIterations();
        for (int i = 0;  i < maxIter && monthsAhead <= maxMonthsAhead;  ++i) {
          int months = startMonth - 1 + monthsAhead;
          nextDate = getFirstDateInMonth(QDate(startYear + months/12, months%12 + 1, 1));
          if (nextDate.isValid())
            break;
          monthsAhead += rFreq;
        }
      }
      break;
    }
    case rYearlyMonth:
    case rYearlyPos:
    case rYearlyDay: {
      int startYear  = dStart.year();
      int yearsAhead = earliestDate.year() - startYear;
      int notThisYear = yearsAhead % rFreq;   // zero if this year is a recurring year
      yearsAhead -= notThisYear;              // latest year which recurred
      // Check for the first later date in the current year
      if (!notThisYear)
        nextDate = getFirstDateInYear(earliestDate);
      // Check for a date in the next scheduled year
      if (!nextDate.isValid()) {
        /* Check for a date in the next scheduled year.
         * The next check may fail if, for example, it's the 29th of February or the 5th
		 * Monday, so limit the number of iterations.
         */
		QDate end = (rDuration >= 0) ? endDate() : MAX_DATE;
        int maxYear = end.year();
        startYear += yearsAhead + rFreq;
        int maxIter = maxIterations();
        for (int i = 0;  i < maxIter && startYear <= maxYear;  ++i) {
          nextDate = getFirstDateInYear(QDate(startYear, 1, 1));
          if (nextDate.isValid())
            break;
          startYear += rFreq;
        }
      }
      break;
    }
    case rNone:
    default:
      return QDate();
  }

  if (rDuration >= 0 && nextDate.isValid()) {
    // Check that the date found is within the range of the recurrence
    QDate end = endDate();
    if ( nextDate > end )
      return QDate();
    if (last  &&  nextDate == end)
      *last = true;
  }

  return nextDate;
}

/* Get the date of the last previous recurrence, before the specified date.
 * Reply = date of previous recurrence, or invalid date if none.
 */
QDate Recurrence::getPreviousDateNoTime(const QDate &afterDate, bool *last) const
{
  if (last)
    *last = false;
  QDate dStart = mRecurStart.date();
  QDate latestDate = afterDate.addDays(-1);
  if (latestDate < dStart)
    return QDate();
  QDate prevDate;

  switch (recurs) {
    case rDaily:
      prevDate = dStart.addDays((dStart.daysTo(latestDate) / rFreq) * rFreq);
      break;

    case rWeekly: {
      QDate start = dStart.addDays(-((dStart.dayOfWeek() - rWeekStart + 7)%7));  // start of week for dStart
      int latestDayOfWeek = latestDate.dayOfWeek();
      int weeksAhead = start.daysTo(latestDate) / 7;
      int notThisWeek = weeksAhead % rFreq;    // zero if this week is a recurring week
      weeksAhead -= notThisWeek;               // latest week which recurred
      int weekday = 0;
      // First check for any previous day this week, if this week is a recurring week
      if (!notThisWeek)
        weekday = getLastDayInWeek(latestDayOfWeek);
      // Check for a day in the previous scheduled week
      if (!weekday) {
        if (!notThisWeek)
          weeksAhead -= rFreq;
        int weekEnd = (rWeekStart + 5)%7 + 1;
        weekday = getLastDayInWeek(weekEnd);
      }
      if (weekday)
        prevDate = start.addDays(weeksAhead*7 + weekday - 1);
      break;
    }
    case rMonthlyDay:
    case rMonthlyPos: {
      int startYear  = dStart.year();
      int startMonth = dStart.month();     // 1..12
      int latestYear = latestDate.year();
      int monthsAhead = (latestYear - startYear)*12 + latestDate.month() - startMonth;
      int notThisMonth = monthsAhead % rFreq;    // zero if this month is a recurring month
      monthsAhead -= notThisMonth;               // latest month which recurred
      // Check for the last earlier day in the current month
      if (!notThisMonth)
        prevDate = getLastDateInMonth(latestDate);
      if (!prevDate.isValid()) {
        /* Check for a day in the previous scheduled month.
         * The next check may fail if, for example, it's the 31st day of the month
         * or the 5th Monday, and the month being checked is February or a 30-day month,
         * so limit the number of iterations.
         */
        if (!notThisMonth)
          monthsAhead -= rFreq;
        int maxIter = maxIterations();
        for (int i = 0;  i < maxIter && monthsAhead >= 0;  ++i) {
          int months = startMonth + monthsAhead;   // get the zero-based month after the one that recurs
          prevDate = getLastDateInMonth(QDate(startYear + months/12, months%12 + 1, 1).addDays(-1));
          if (prevDate.isValid())
            break;
          monthsAhead -= rFreq;
        }
      }
      break;
    }
    case rYearlyMonth:
    case rYearlyPos:
    case rYearlyDay: {
      int startYear  = dStart.year();
      int yearsAhead = latestDate.year() - startYear;
      int notThisYear = yearsAhead % rFreq;   // zero if this year is a recurring year
      yearsAhead -= notThisYear;              // latest year which recurred
      // Check for the last earlier date in the current year
      if (!notThisYear)
        prevDate = getLastDateInYear(latestDate);
      if (!prevDate.isValid()) {
        /* Check for a date in the previous scheduled year.
         * The next check may fail if, for example, it's the 29th of February or the 5th
		 * Monday, so limit the number of iterations.
         */
        if (!notThisYear)
          yearsAhead -= rFreq;
        int maxIter = maxIterations();
        for (int i = 0;  i < maxIter && yearsAhead >= 0;  ++i) {
          prevDate = getLastDateInYear(QDate(startYear + yearsAhead, 12, 31));
          if (prevDate.isValid())
            break;
          yearsAhead -= rFreq;
        }
      }
      break;
    }
    case rNone:
    default:
      return QDate();
  }

  if (prevDate.isValid()) {
    // Check that the date found is within the range of the recurrence
    if (prevDate < dStart)
      return QDate();
    if (rDuration >= 0) {
      QDate end = endDate();
      if (prevDate >= end) {
        if (last)
          *last = true;
        return end;
      }
    }
  }
  return prevDate;
}

int Recurrence::maxIterations() const
{
  /* Find the maximum number of iterations which may be needed to reach the
   * next actual occurrence of a monthly or yearly recurrence.
   * More than one iteration may be needed if, for example, it's the 29th February,
   * the 31st day of the month or the 5th Monday, and the month being checked is
   * February or a 30-day month.
   * The following recurrences may never occur:
   * - For rMonthlyDay: if the frequency is a whole number of years.
   * - For rMonthlyPos: if the frequency is an even whole number of years.
   * - For rYearlyDay, rYearlyMonth: if the frequeny is a multiple of 4 years.
   * - For rYearlyPos: if the frequency is an even number of years.
   * The maximum number of iterations needed, assuming that it does actually occur,
   * was found empirically.
   */
  switch (recurs) {
    case rMonthlyDay:
      return (rFreq % 12) ? 6 : 8;

    case rMonthlyPos:
      if (rFreq % 12 == 0) {
        // Some of these frequencies may never occur
        return (rFreq % 84 == 0) ? 364         // frequency = multiple of 7 years
             : (rFreq % 48 == 0) ? 7           // frequency = multiple of 4 years
             : (rFreq % 24 == 0) ? 14 : 28;    // frequency = multiple of 2 or 1 year
      }
      // All other frequencies will occur sometime
      if (rFreq > 120)
        return 364;    // frequencies of > 10 years will hit the date limit first
      switch (rFreq) {
        case 23:   return 50;
        case 46:   return 38;
        case 56:   return 138;
        case 66:   return 36;
        case 89:   return 54;
        case 112:  return 253;
        default:   return 25;       // most frequencies will need < 25 iterations
      }

    case rYearlyMonth:
    case rYearlyDay:
      return 8;          // only 29th Feb or day 366 will need more than one iteration

    case rYearlyPos:
      if (rFreq % 7 == 0)
        return 364;    // frequencies of a multiple of 7 years will hit the date limit first
      if (rFreq % 2 == 0) {
        // Some of these frequencies may never occur
        return (rFreq % 4 == 0) ? 7 : 14;    // frequency = even number of years
      }
      return 28;
  }
  return 1;
}

void Recurrence::setDailySub(short type, int freq, int duration)
{
  mUseCachedEndDT = false;
  recurs = type;
  rFreq = freq;
  rDuration = duration;
  rMonthPositions.clear();
  rMonthDays.clear();
  rYearNums.clear();
  if (type != rDaily)
    mFloats = false;     // sub-daily types can't be floating

  if (mParent) mParent->updated();
}

void Recurrence::setYearly_(short type, Feb29Type feb29type, int freq, int duration)
{
  mUseCachedEndDT = false;
  recurs = type;
  if (mCompatVersion < 310 && type == rYearlyDay) {
    mCompatRecurs = rYearlyDay;
    recurs = rYearlyMonth;      // convert old yearly-by-day to yearly-by-month
    feb29type = rMar1;          // retain the same day number in the year
  }

  mFeb29YearlyType = feb29type;
  rFreq = freq;
  rDuration = duration;
  if (type != rYearlyPos)
    rMonthPositions.clear();
  rMonthDays.clear();
  if (mParent) mParent->updated();
}

int Recurrence::recurCalc(PeriodFunc func, QDateTime &endtime) const
{
  QDate enddate = endtime.date();
  switch (func) {
    case END_DATE_AND_COUNT:
      if (rDuration < 0) {
        endtime = QDateTime();
        return 0;    // infinite recurrence
      }
      if (rDuration == 0) {
        endtime = rEndDateTime;
        func = COUNT_TO_DATE;
      }
      break;
    case COUNT_TO_DATE:
      // Count recurrences up to and including the specified date/time.
      if (endtime < mRecurStart)
        return 0;
      if (rDuration == 0 && endtime > rEndDateTime)
        enddate = rEndDateTime.date();
      else if (!mFloats && mRecurStart.time() > endtime.time())
        enddate = enddate.addDays(-1);
      break;
    case NEXT_AFTER_DATE:
      // Find next recurrence AFTER endtime
      if (endtime < mRecurStart) {
        endtime = mRecurStart;
        return 1;
      }
      if (rDuration == 0 && endtime >= rEndDateTime) {
        endtime = QDateTime();
        return 0;
      }
      if (!mFloats && mRecurStart.time() > endtime.time())
        enddate = enddate.addDays(-1);
      break;
    default:
      endtime = QDateTime();
      return 0;
  }

  int count = 0;     // default = error
  bool timed = false;
  switch (recurs) {
    case rMinutely:
      timed = true;
      count = secondlyCalc(func, endtime, rFreq*60);
      break;
    case rHourly:
      timed = true;
      count = secondlyCalc(func, endtime, rFreq*3600);
      break;
    case rDaily:
      count = dailyCalc(func, enddate);
      break;
    case rWeekly:
      count = weeklyCalc(func, enddate);
      break;
    case rMonthlyPos:
    case rMonthlyDay:
      count = monthlyCalc(func, enddate);
      break;
    case rYearlyMonth:
      count = yearlyMonthCalc(func, enddate);
      break;
    case rYearlyPos:
      count = yearlyPosCalc(func, enddate);
      break;
    case rYearlyDay:
      count = yearlyDayCalc(func, enddate);
      break;
    default:
      break;
  }

  switch (func) {
    case END_DATE_AND_COUNT:
    case NEXT_AFTER_DATE:
      if (count == 0)
        endtime = QDateTime();
      else if (!timed) {
        endtime.setDate(enddate);
        endtime.setTime(mRecurStart.time());
      }
      break;
    case COUNT_TO_DATE:
      break;
  }
  return count;
}

int Recurrence::recurCalc(PeriodFunc func, QDate &enddate) const
{
  QDateTime endtime(enddate, QTime(23,59,59));
  switch (func) {
    case END_DATE_AND_COUNT:
      if (rDuration < 0) {
        enddate = QDate();
        return 0;    // infinite recurrence
      }
      if (rDuration == 0) {
        enddate = rEndDateTime.date();
        func = COUNT_TO_DATE;
      }
      break;
    case COUNT_TO_DATE:
      // Count recurrences up to and including the specified date.
      if (enddate < mRecurStart.date())
        return 0;
      if (rDuration == 0 && enddate > rEndDateTime.date()) {
        enddate = rEndDateTime.date();
        endtime.setDate(enddate);
      }
      break;
    case NEXT_AFTER_DATE:
      if (enddate < mRecurStart.date()) {
        enddate = mRecurStart.date();
        return 1;
      }
      if (rDuration == 0 && enddate >= rEndDateTime.date()) {
        enddate = QDate();
        return 0;
      }
      break;
    default:
      enddate = QDate();
      return 0;
  }

  int count = 0;     // default = error
  bool timed = false;
  switch (recurs) {
    case rMinutely:
      timed = true;
      count = secondlyCalc(func, endtime, rFreq*60);
      break;
    case rHourly:
      timed = true;
      count = secondlyCalc(func, endtime, rFreq*3600);
      break;
    case rDaily:
      count = dailyCalc(func, enddate);
      break;
    case rWeekly:
      count = weeklyCalc(func, enddate);
      break;
    case rMonthlyPos:
    case rMonthlyDay:
      count = monthlyCalc(func, enddate);
      break;
    case rYearlyMonth:
      count = yearlyMonthCalc(func, enddate);
      break;
    case rYearlyPos:
      count = yearlyPosCalc(func, enddate);
      break;
    case rYearlyDay:
      count = yearlyDayCalc(func, enddate);
      break;
    default:
      break;
  }

  switch (func) {
    case END_DATE_AND_COUNT:
    case NEXT_AFTER_DATE:
      if (count == 0)
        endtime = QDate();
      else if (timed)
        enddate = endtime.date();
      break;
    case COUNT_TO_DATE:
      break;
  }
  return count;
}

/* Find count and, depending on 'func', the end date/time of a secondly recurrence.
 * Reply = total number of occurrences up to 'endtime', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'endtime' is updated to the
 * recurrence end date/time.
 */
int Recurrence::secondlyCalc(PeriodFunc func, QDateTime &endtime, int freq) const
{
  switch (func) {
    case END_DATE_AND_COUNT:
      endtime = mRecurStart.addSecs((rDuration - 1) * freq);
      return rDuration;
    case COUNT_TO_DATE: {
      int n = mRecurStart.secsTo(endtime)/freq + 1;
      if (rDuration > 0 && n > rDuration)
        return rDuration;
      return n;
    }
    case NEXT_AFTER_DATE: {
      int count = mRecurStart.secsTo(endtime) / freq + 2;
      if (rDuration > 0 && count > rDuration)
        return 0;
      endtime = mRecurStart.addSecs((count - 1)*freq);
      return count;
    }
  }
  return 0;
}

/* Find count and, depending on 'func', the end date of a daily recurrence.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 */
int Recurrence::dailyCalc(PeriodFunc func, QDate &enddate) const
{
  QDate dStart = mRecurStart.date();
  switch (func) {
    case END_DATE_AND_COUNT:
      enddate = dStart.addDays((rDuration - 1) * rFreq);
      return rDuration;
    case COUNT_TO_DATE: {
      int n = dStart.daysTo(enddate)/rFreq + 1;
      if (rDuration > 0 && n > rDuration)
        return rDuration;
      return n;
    }
    case NEXT_AFTER_DATE: {
      int count = dStart.daysTo(enddate) / rFreq + 2;
      if (rDuration > 0 && count > rDuration)
        return 0;
      enddate = dStart.addDays((count - 1)*rFreq);
      return count;
    }
  }
  return 0;
}

/* Find count and, depending on 'func', the end date of a weekly recurrence.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 */
int Recurrence::weeklyCalc(PeriodFunc func, QDate &enddate) const
{
  int daysPerWeek = 0;
  for (int i = 0;  i < 7;  ++i) {
    if (rDays.testBit((uint)i))
      ++daysPerWeek;
  }
  if (!daysPerWeek)
    return 0;     // there are no days to recur on

  switch (func) {
    case END_DATE_AND_COUNT:
      return weeklyCalcEndDate(enddate, daysPerWeek);
    case COUNT_TO_DATE:
      return weeklyCalcToDate(enddate, daysPerWeek);
    case NEXT_AFTER_DATE:
      return weeklyCalcNextAfter(enddate, daysPerWeek);
  }
  return 0;
}

int Recurrence::weeklyCalcEndDate(QDate &enddate, int daysPerWeek) const
{
  int startDayOfWeek = mRecurStart.date().dayOfWeek();     // 1..7
  int countGone = 0;
  int daysGone = 0;
  uint countTogo = rDuration;
  if (startDayOfWeek != rWeekStart) {
    // Check what remains of the start week
    for (int i = startDayOfWeek - 1;  i != rWeekStart - 1;  i = (i + 1) % 7) {
	  ++daysGone;
      if (rDays.testBit((uint)i)) {
        ++countGone;
        if (--countTogo == 0)
          break;
      }
    }
    daysGone += 7 * (rFreq - 1);
  }
  if (countTogo) {
    // Skip the remaining whole weeks
    // Leave at least 1 recurrence remaining, in order to get its date
    int wholeWeeks = (countTogo - 1) / daysPerWeek;
    daysGone += wholeWeeks * 7 * rFreq;
    countGone += wholeWeeks * daysPerWeek;
    countTogo -= wholeWeeks * daysPerWeek;
    // Check the last week in the recurrence
    for (int i = rWeekStart - 1;  ;  i = (i + 1) % 7) {
	  ++daysGone;
      if (rDays.testBit((uint)i)) {
        ++countGone;
        if (--countTogo == 0)
          break;
      }
    }
  }
  enddate = mRecurStart.date().addDays(daysGone);
  return countGone;
}

int Recurrence::weeklyCalcToDate(const QDate &enddate, int daysPerWeek) const
{
  QDate dStart = mRecurStart.date();
  int startDayOfWeek = dStart.dayOfWeek();     // 1..7
  int countGone = 0;
  int daysGone  = 0;
  int totalDays = dStart.daysTo(enddate) + 1;
  int countMax  = (rDuration > 0) ? rDuration : INT_MAX;

  if (startDayOfWeek != rWeekStart) {
    // Check what remains of the start week
    for (int i = startDayOfWeek - 1;  i != rWeekStart - 1;  i = (i + 1) % 7) {
      if (rDays.testBit((uint)i)) {
        if (++countGone >= countMax)
          return countMax;
      }
      if (++daysGone == totalDays)
        return countGone;
    }
    daysGone += 7 * (rFreq - 1);
    if (daysGone >= totalDays)
      return countGone;
  }
  // Skip the remaining whole weeks
  int wholeWeeks = (totalDays - daysGone) / 7;
  countGone += (wholeWeeks / rFreq) * daysPerWeek;
  if (countGone >= countMax)
    return countMax;
  daysGone += wholeWeeks * 7;
  if (daysGone >= totalDays     // have we reached the end date?
  ||  wholeWeeks % rFreq)       // is end week a recurrence week?
    return countGone;

  // Check the last week in the recurrence
  for (int i = rWeekStart - 1;  ;  i = (i + 1) % 7) {
    if (rDays.testBit((uint)i)) {
      if (++countGone >= countMax)
        return countMax;
    }
    if (++daysGone == totalDays)
      return countGone;
  }
  return countGone;
}

int Recurrence::weeklyCalcNextAfter(QDate &enddate, int daysPerWeek) const
{
  QDate dStart = mRecurStart.date();
  int  startDayOfWeek = dStart.dayOfWeek();     // 1..7
  int  totalDays = dStart.daysTo(enddate) + 1;
  uint countTogo = (rDuration > 0) ? rDuration : UINT_MAX;
  int  countGone = 0;
  int  daysGone = 0;
  int recurWeeks;

  if (startDayOfWeek != rWeekStart) {
    // Check what remains of the start week
    for (int i = startDayOfWeek - 1;  i != rWeekStart - 1;  i = (i + 1) % 7) {
      ++daysGone;
      if (rDays.testBit((uint)i)) {
        ++countGone;
        if (daysGone > totalDays)
          goto ex;
        if (--countTogo == 0)
          return 0;
      }
    }
    daysGone += 7 * (rFreq - 1);
  }

  // Skip the remaining whole weeks
  recurWeeks = (totalDays - daysGone) / (7 * rFreq);
  if (recurWeeks) {
    int n = recurWeeks * daysPerWeek;
    if (static_cast<uint>(n) > countTogo)
        return 0;     // reached end of recurrence
    countGone += n;
    countTogo -= n;
   daysGone += recurWeeks * 7 * rFreq;
  }

  // Check the last week or two in the recurrence
  for ( ; ; ) {
    for (int i = rWeekStart - 1;  ;  i = (i + 1) % 7) {
      ++daysGone;
      if (rDays.testBit((uint)i)) {
        ++countGone;
        if (daysGone > totalDays)
          goto ex;
        if (--countTogo == 0)
          return 0;
      }
    }
    daysGone += 7 * (rFreq - 1);
  }
ex:
  enddate = dStart.addDays(daysGone);
  return countGone;
}

/* Find count and, depending on 'func', the end date of a monthly recurrence.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 */
class Recurrence::MonthlyData
{
  public:
    const Recurrence *recurrence;
    int               year;          // current year
    int               month;         // current month 0..11
    int               day;           // current day of month 1..31
    bool              varies;        // true if recurring days vary between different months

  private:
    QValueList<int>   days28, days29, days30, days31;   // recurring days in months of each length
    QValueList<int>  *recurDays[4];

  public:
    MonthlyData(const Recurrence* r, const QDate &date)
             : recurrence(r), year(date.year()), month(date.month()-1), day(date.day())
             { recurDays[0] = &days28;
               recurDays[1] = &days29;
               recurDays[2] = &days30;
               recurDays[3] = &days31;
               varies = (recurrence->doesRecur() == rMonthlyPos)
                        ? true : recurrence->getMonthlyDayDays(days31, 31);
             }
    const QValueList<int>* dayList() const {
            if (!varies)
              return &days31;
            QDate startOfMonth(year, month + 1, 1);
            int daysInMonth = startOfMonth.daysInMonth();
            QValueList<int>* days = recurDays[daysInMonth - 28];
            if (recurrence->doesRecur() == rMonthlyPos)
              recurrence->getMonthlyPosDays(*days, daysInMonth, startOfMonth.dayOfWeek());
            else if (days->isEmpty())
              recurrence->getMonthlyDayDays(*days, daysInMonth);
            return days;
    }
    int    yearMonth() const    { return year*12 + month; }
    void   addMonths(int diff)  { month += diff;  year += month / 12;  month %= 12; }
    QDate  date() const         { return QDate(year, month + 1, day); }
};

int Recurrence::monthlyCalc(PeriodFunc func, QDate &enddate) const
{
  if ( (recurs == rMonthlyPos && rMonthPositions.isEmpty() )
       || ( recurs == rMonthlyDay && rMonthDays.isEmpty() ) )
    return 0;

  MonthlyData data(this, mRecurStart.date());
  switch (func) {
    case END_DATE_AND_COUNT:
      return monthlyCalcEndDate(enddate, data);
    case COUNT_TO_DATE:
      return monthlyCalcToDate(enddate, data);
    case NEXT_AFTER_DATE:
      return monthlyCalcNextAfter(enddate, data);
  }
  return 0;
}

int Recurrence::monthlyCalcEndDate(QDate &enddate, MonthlyData &data) const
{
  uint countTogo = rDuration;
  int  countGone = 0;
  QValueList<int>::ConstIterator it;
  const QValueList<int>* days = data.dayList();

  if (data.day > 1) {
    // Check what remains of the start month
    for (it = days->begin();  it != days->end();  ++it) {
      if (*it >= data.day) {
        ++countGone;
        if (--countTogo == 0) {
          data.day = *it;
          break;
        }
      }
    }
    if (countTogo) {
      data.day = 1;
      data.addMonths(rFreq);
    }
  }
  if (countTogo) {
    if (data.varies) {
      // The number of recurrence days varies from month to month,
      // so we need to check month by month.
      for ( ; ; ) {
        days = data.dayList();
        uint n = days->count();    // number of recurrence days in this month
        if (n >= countTogo)
          break;
        countTogo -= n;
        countGone += n;
        data.addMonths(rFreq);
      }
    } else {
      // The number of recurrences is the same every month,
      // so skip the month-by-month check.
      // Skip the remaining whole months, but leave at least
      // 1 recurrence remaining, in order to get its date.
      int daysPerMonth = days->count();
      int wholeMonths = (countTogo - 1) / daysPerMonth;
      data.addMonths(wholeMonths * rFreq);
      countGone += wholeMonths * daysPerMonth;
      countTogo -= wholeMonths * daysPerMonth;
    }
    if (countTogo) {
      // Check the last month in the recurrence
      for (it = days->begin();  it != days->end();  ++it) {
        ++countGone;
        if (--countTogo == 0) {
          data.day = *it;
          break;
        }
      }
    }
  }
  enddate = data.date();
  return countGone;
}

int Recurrence::monthlyCalcToDate(const QDate &enddate, MonthlyData &data) const
{
  int countGone = 0;
  int countMax  = (rDuration > 0) ? rDuration : INT_MAX;
  int endYear  = enddate.year();
  int endMonth = enddate.month() - 1;     // zero-based
  int endDay   = enddate.day();
  int endYearMonth = endYear*12 + endMonth;
  QValueList<int>::ConstIterator it;
  const QValueList<int>* days = data.dayList();

  if (data.day > 1) {
    // Check what remains of the start month
    for (it = days->begin();  it != days->end();  ++it) {
      if (*it >= data.day) {
        if (data.yearMonth() == endYearMonth && *it > endDay)
          return countGone;
        if (++countGone >= countMax)
          return countMax;
      }
    }
    data.day = 1;
    data.addMonths(rFreq);
  }

  if (data.varies) {
    // The number of recurrence days varies from month to month,
    // so we need to check month by month.
    while (data.yearMonth() < endYearMonth) {
      countGone += data.dayList()->count();
      if (countGone >= countMax)
        return countMax;
      data.addMonths(rFreq);
    }
    days = data.dayList();
  } else {
    // The number of recurrences is the same every month,
    // so skip the month-by-month check.
    // Skip the remaining whole months.
    int daysPerMonth = days->count();
    int wholeMonths = endYearMonth - data.yearMonth();
    countGone += (wholeMonths / rFreq) * daysPerMonth;
    if (countGone >= countMax)
      return countMax;
    if (wholeMonths % rFreq)
      return countGone;      // end year isn't a recurrence year
    data.year  = endYear;
    data.month = endMonth;
  }

  // Check the last month in the recurrence
  for (it = days->begin();  it != days->end();  ++it) {
    if (*it > endDay)
      return countGone;
    if (++countGone >= countMax)
      return countMax;
  }
  return countGone;
}

int Recurrence::monthlyCalcNextAfter(QDate &enddate, MonthlyData &data) const
{
  uint countTogo = (rDuration > 0) ? rDuration : UINT_MAX;
  int countGone = 0;
  int endYear = enddate.year();
  int endDay  = enddate.day();
  int endYearMonth = endYear*12 + enddate.month() - 1;
  QValueList<int>::ConstIterator it;
  const QValueList<int>* days = data.dayList();

  if (data.day > 1) {
    // Check what remains of the start month
    for (it = days->begin();  it != days->end();  ++it) {
      if (*it >= data.day) {
        ++countGone;
        if (data.yearMonth() == endYearMonth && *it > endDay) {
          data.day = *it;
          goto ex;
        }
        if (--countTogo == 0)
          return 0;
      }
    }
    data.day = 1;
    data.addMonths(rFreq);
  }

  if (data.varies) {
    // The number of recurrence days varies from month to month,
    // so we need to check month by month.
    while (data.yearMonth() <= endYearMonth) {
      days = data.dayList();
      uint n = days->count();    // number of recurrence days in this month
      if (data.yearMonth() == endYearMonth && days->last() > endDay)
        break;
      if (n >= countTogo)
        return 0;
      countGone += n;
      countTogo -= n;
      data.addMonths(rFreq);
    }
    days = data.dayList();
  } else {
    // The number of recurrences is the same every month,
    // so skip the month-by-month check.
    // Skip the remaining whole months to at least end year/month.
    int daysPerMonth = days->count();
    int elapsed = endYearMonth - data.yearMonth();
    int recurMonths = (elapsed + rFreq - 1) / rFreq;
    if (elapsed % rFreq == 0  &&  days->last() <= endDay)
      ++recurMonths;    // required month is after endYearMonth
    if (recurMonths) {
      int n = recurMonths * daysPerMonth;
      if (static_cast<uint>(n) > countTogo)
        return 0;     // reached end of recurrence
      countTogo -= n;
      countGone += n;
      data.addMonths(recurMonths * rFreq);
    }
  }

  // Check the last month in the recurrence
  for (it = days->begin();  it != days->end();  ++it) {
    ++countGone;
    if (data.yearMonth() > endYearMonth  ||  *it > endDay) {
      data.day = *it;
      break;
    }
    if (--countTogo == 0)
      return 0;
  }
ex:
  enddate = data.date();
  return countGone;
}


/* Find count and, depending on 'func', the end date of an annual recurrence by date.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 *
 * WARNING: These methods currently do not cater for day of month < -28
 *          (which would need different months to be treated differently).
 */
class Recurrence::YearlyMonthData
{
  public:
    const Recurrence *recurrence;
    int               year;          // current year
    int               month;         // current month 1..12
    int               day;           // current day of month 1..31
    bool              leapyear;      // true if February 29th recurs and current year is a leap year
    bool              feb29;         // true if February 29th recurs

  private:
    QValueList<int>   months;        // recurring months in non-leap years  1..12
    QValueList<int>   leapMonths;    // recurring months in leap years  1..12

  public:
    YearlyMonthData(const Recurrence* r, const QDate &date, int d)
          : recurrence(r), year(date.year()), month(date.month()), day(d ? d : date.day())
          { feb29 = recurrence->getYearlyMonthMonths(day, months, leapMonths);
            leapyear = feb29 && QDate::leapYear(year);
          }
    const QValueList<int>* monthList() const
                         { return leapyear ? &leapMonths : &months; }
    const QValueList<int>* leapMonthList() const  { return &leapMonths; }
    QDate            date() const  { if (day > 0) return QDate(year, month, day);
                                     return QDate(year, month, QDate(year, month, 1).daysInMonth() + day + 1);
                                   }
};

int Recurrence::yearlyMonthCalc(PeriodFunc func, QDate &enddate) const
{
  if (rYearNums.isEmpty())
    return 0;
  YearlyMonthData data(this, mRecurStart.date(), (rMonthDays.count() ? *rMonthDays.getFirst() : 0));
  switch (func) {
    case END_DATE_AND_COUNT:
      return yearlyMonthCalcEndDate(enddate, data);
    case COUNT_TO_DATE:
      return yearlyMonthCalcToDate(enddate, data);
    case NEXT_AFTER_DATE:
      return yearlyMonthCalcNextAfter(enddate, data);
  }
  return 0;
}

// Find total count and end date of an annual recurrence by date.
// Reply = total number of occurrences.
int Recurrence::yearlyMonthCalcEndDate(QDate &enddate, YearlyMonthData &data) const
{
  uint countTogo = rDuration;
  int  countGone = 0;
  QValueList<int>::ConstIterator it;
  const QValueList<int>* mons = data.monthList();   // get recurring months for this year

  if (data.month > 1) {
    // Check what remains of the start year
    for (it = mons->begin();  it != mons->end();  ++it) {
      if (*it >= data.month) {
        ++countGone;
        if (--countTogo == 0) {
          data.month = *it;
          if (data.month == 2 && data.feb29 && !data.leapyear) {
            // The recurrence should end on February 29th, but it's a non-leap year
            switch (mFeb29YearlyType) {
              case rFeb28:
                data.day = 28;
                break;
              case rMar1:
                data.month = 3;
                data.day   = 1;
                break;
              case rFeb29:
                break;
            }
          }
          break;
        }
      }
    }
    if (countTogo) {
      data.month = 1;
      data.year += rFreq;
    }
  }
  if (countTogo) {
    if (data.feb29 && mFeb29YearlyType == rFeb29) {
      // The number of recurrences is different on leap years,
      // so check year-by-year.
      for ( ; ; ) {
        mons = data.monthList();
        uint n = mons->count();
        if (n >= countTogo)
          break;
        countTogo -= n;
        countGone += n;
        data.year += rFreq;
      }
    } else {
      // The number of recurrences is the same every year,
      // so skip the year-by-year check.
      // Skip the remaining whole years, but leave at least
      // 1 recurrence remaining, in order to get its date.
      int monthsPerYear = mons->count();
      int wholeYears = (countTogo - 1) / monthsPerYear;
      data.year += wholeYears * rFreq;
      countGone += wholeYears * monthsPerYear;
      countTogo -= wholeYears * monthsPerYear;
    }
    if (countTogo) {
      // Check the last year in the recurrence
      for (it = mons->begin();  it != mons->end();  ++it) {
        ++countGone;
        if (--countTogo == 0) {
          data.month = *it;
          if (data.month == 2 && data.feb29 && !QDate::leapYear(data.year)) {
            // The recurrence should end on February 29th, but it's a non-leap year
            switch (mFeb29YearlyType) {
              case rFeb28:
                data.day = 28;
                break;
              case rMar1:
                data.month = 3;
                data.day   = 1;
                break;
              case rFeb29:
                break;
            }
          }
          break;
        }
      }
    }
  }
  enddate = data.date();
  return countGone;
}

// Find count of an annual recurrence by date.
// Reply = total number of occurrences up to 'enddate'.
int Recurrence::yearlyMonthCalcToDate(const QDate &enddate, YearlyMonthData &data) const
{
  int countGone = 0;
  int countMax  = (rDuration > 0) ? rDuration : INT_MAX;
  int endYear  = enddate.year();
  int endMonth = enddate.month();
  int endDay   = enddate.day();
  if (data.day < 0) {
    // The end day of the month is relative to the end of the month.
    if (endDay < enddate.daysInMonth() + data.day + 1) {
      if (--endMonth == 0) {
        endMonth = 12;
        --endYear;
      }
    }
  }
  else if (endDay < data.day) {
    /* The end day of the month is earlier than the recurrence day of the month.
     * If Feb 29th recurs and:
     * 1) it recurs on Feb 28th in non-leap years, don't adjust the end month
     *    if enddate is Feb 28th on a non-leap year.
     * 2) it recurs on Mar 1st in non-leap years, allow the end month to be
     *    adjusted to February, to simplify calculations.
     */
    if (data.feb29  &&  !QDate::leapYear(endYear)
    &&  mFeb29YearlyType == rFeb28  &&  endDay == 28  &&  endMonth == 2) {
    }
    else if (--endMonth == 0) {
      endMonth = 12;
      --endYear;
    }
  }
  QValueList<int>::ConstIterator it;
  const QValueList<int>* mons = data.monthList();

  if (data.month > 1) {
    // Check what remains of the start year
    for (it = mons->begin();  it != mons->end();  ++it) {
      if (*it >= data.month) {
        if (data.year == endYear && *it > endMonth)
          return countGone;
        if (++countGone >= countMax)
          return countMax;
      }
    }
    data.month = 1;
    data.year += rFreq;
  }
  if (data.feb29 && mFeb29YearlyType == rFeb29) {
    // The number of recurrences is different on leap years,
    // so check year-by-year.
    while (data.year < endYear) {
      countGone += data.monthList()->count();
      if (countGone >= countMax)
        return countMax;
      data.year += rFreq;
    }
    mons = data.monthList();
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years.
    int monthsPerYear = mons->count();
    int wholeYears = endYear - data.year;
    countGone += (wholeYears / rFreq) * monthsPerYear;
    if (countGone >= countMax)
      return countMax;
    if (wholeYears % rFreq)
      return countGone;      // end year isn't a recurrence year
    data.year = endYear;
  }

  // Check the last year in the recurrence
  for (it = mons->begin();  it != mons->end();  ++it) {
    if (*it > endMonth)
      return countGone;
    if (++countGone >= countMax)
      return countMax;
  }
  return countGone;
}

// Find count and date of first recurrence after 'enddate' of an annual recurrence by date.
// Reply = total number of occurrences up to 'enddate'.
int Recurrence::yearlyMonthCalcNextAfter(QDate &enddate, YearlyMonthData &data) const
{
  uint countTogo = (rDuration > 0) ? rDuration : UINT_MAX;
  int  countGone = 0;
  int endYear  = enddate.year();
  int endMonth = enddate.month();
  int endDay   = enddate.day();
  bool mar1TooEarly = false;
  bool feb28ok      = false;
  if (data.day < 0) {
    // The end day of the month is relative to the end of the month.
    if (endDay < enddate.daysInMonth() + data.day + 1) {
      if (--endMonth == 0) {
        endMonth = 12;
        --endYear;
      }
    }
  }
  else if (endDay < data.day) {
    if (data.feb29 && mFeb29YearlyType == rMar1 && endMonth == 3)
      mar1TooEarly = true;
    if (data.feb29 && mFeb29YearlyType == rFeb28 && endMonth == 2 && endDay == 28)
      feb28ok = true;
    else if (--endMonth == 0) {
      endMonth = 12;
      --endYear;
    }
  }
  QValueList<int>::ConstIterator it;
  const QValueList<int>* mons = data.monthList();

  if (data.month > 1) {
    // Check what remains of the start year
    for (it = mons->begin();  it != mons->end();  ++it) {
      if (*it >= data.month) {
        ++countGone;
        if (data.year == endYear
        &&  (   *it > endMonth && (*it > 3 || !mar1TooEarly)
             || *it == 2 && feb28ok && data.leapyear)) {
          if (*it == 2 && data.feb29 && !data.leapyear) {
            // The next recurrence should be on February 29th, but it's a non-leap year
            switch (mFeb29YearlyType) {
              case rFeb28:
                data.month = 2;
                data.day   = 28;
                break;
              case rMar1:
                data.month = 3;
                data.day   = 1;
                break;
              case rFeb29:   // impossible in this context!
                break;
            }
          }
          else
            data.month = *it;
          goto ex;
        }
        if (--countTogo == 0)
          return 0;
      }
    }
    data.month = 1;
    data.year += rFreq;
  }

  if (data.feb29 && mFeb29YearlyType == rFeb29) {
    // The number of recurrences is different on leap years,
    // so check year-by-year.
    while (data.year <= endYear) {
      mons = data.monthList();
      if (data.year == endYear && mons->last() > endMonth)
        break;
      uint n = mons->count();
      if (n >= countTogo)
        break;
      countTogo -= n;
      countGone += n;
      data.year += rFreq;
    }
    mons = data.monthList();
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years to at least endYear.
    int monthsPerYear = mons->count();
    int recurYears = (endYear - data.year + rFreq - 1) / rFreq;
    if ((endYear - data.year)%rFreq == 0
    &&  mons->last() <= endMonth)
      ++recurYears;    // required year is after endYear
    if (recurYears) {
      int n = recurYears * monthsPerYear;
      if (static_cast<uint>(n) > countTogo)
        return 0;     // reached end of recurrence
      countTogo -= n;
      countGone += n;
      data.year += recurYears * rFreq;
    }
  }

  // Check the last year in the recurrence
  for (it = mons->begin();  it != mons->end();  ++it) {
    ++countGone;
    if (data.year > endYear
    ||  (   *it > endMonth && (*it > 3 || !mar1TooEarly)
         || *it == 2 && feb28ok && QDate::leapYear(data.year))) {
      if (*it == 2 && data.feb29 && !QDate::leapYear(data.year)) {
        // The next recurrence should be on February 29th, but it's a non-leap year
        switch (mFeb29YearlyType) {
          case rFeb28:
            data.month = 2;
            data.day   = 28;
            break;
          case rMar1:
            data.month = 3;
            data.day   = 1;
            break;
          case rFeb29:   // impossible in this context!
            break;
        }
      }
      else
        data.month = *it;
      break;
    }
    if (--countTogo == 0)
      return 0;
  }
ex:
  enddate = data.date();
  return countGone;
}


/* Find count and, depending on 'func', the end date of an annual recurrence by date.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 */
class Recurrence::YearlyPosData
{
  public:
    const Recurrence *recurrence;
    int               year;          // current year
    int               month;         // current month 1..12
    int               day;           // current day of month 1..31
    int               daysPerMonth;  // number of days which recur each month, or -1 if variable
    int               count;         // number of days which recur each year, or -1 if variable
    bool              varies;        // true if number of days varies from year to year

  private:
    mutable QValueList<int> days;

  public:
    YearlyPosData(const Recurrence* r, const QDate &date)
          : recurrence(r), year(date.year()), month(date.month()), day(date.day()), count(-1)
            { if ((daysPerMonth = r->countMonthlyPosDays()) > 0)
                count = daysPerMonth * r->yearNums().count();
              varies = (daysPerMonth < 0);
            }
    const QValueList<int>* dayList() const {
            QDate startOfMonth(year, month, 1);
            recurrence->getMonthlyPosDays(days, startOfMonth.daysInMonth(), startOfMonth.dayOfWeek());
            return &days;
    }
    int    yearMonth() const    { return year*12 + month - 1; }
    void   addMonths(int diff)  { month += diff - 1;  year += month / 12;  month = month % 12 + 1; }
    QDate  date() const         { return QDate(year, month, day); }
};

int Recurrence::yearlyPosCalc(PeriodFunc func, QDate &enddate) const
{
  if (rYearNums.isEmpty() || rMonthPositions.isEmpty())
    return 0;
  YearlyPosData data(this, mRecurStart.date());
  switch (func) {
    case END_DATE_AND_COUNT:
      return yearlyPosCalcEndDate(enddate, data);
    case COUNT_TO_DATE:
      return yearlyPosCalcToDate(enddate, data);
    case NEXT_AFTER_DATE:
      return yearlyPosCalcNextAfter(enddate, data);
  }
  return 0;
}

int Recurrence::yearlyPosCalcEndDate(QDate &enddate, YearlyPosData &data) const
{
  uint countTogo = rDuration;
  int  countGone = 0;
  QValueList<int>::ConstIterator id;
  const QValueList<int>* days;

  if (data.month > 1 || data.day > 1) {
    // Check what remains of the start year
    for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
      if (*im.current() >= data.month) {
        // Check what remains of the start month
        if (data.day > 1 || data.varies
        ||  static_cast<uint>(data.daysPerMonth) >= countTogo) {
          data.month = *im.current();
          days = data.dayList();
          for (id = days->begin();  id != days->end();  ++id) {
            if (*id >= data.day) {
              ++countGone;
              if (--countTogo == 0) {
                data.month = *im.current();
                data.day = *id;
                goto ex;
              }
            }
          }
          data.day = 1;
        } else {
          // The number of days per month is constant, so skip
          // the whole month.
          countTogo -= data.daysPerMonth;
          countGone += data.daysPerMonth;
        }
      }
    }
    data.month = 1;
    data.year += rFreq;
  }

  if (data.varies) {
    // The number of recurrences varies from year to year.
    for ( ; ; ) {
      for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
        data.month = *im.current();
        days = data.dayList();
        int n = days->count();
        if (static_cast<uint>(n) >= countTogo) {
          // Check the last month in the recurrence
          for (id = days->begin();  id != days->end();  ++id) {
            ++countGone;
            if (--countTogo == 0) {
              data.day = *id;
              goto ex;
            }
          }
        }
        countTogo -= n;
        countGone += n;
      }
      data.year += rFreq;
    }
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years, but leave at least
    // 1 recurrence remaining, in order to get its date.
    int wholeYears = (countTogo - 1) / data.count;
    data.year += wholeYears * rFreq;
    countGone += wholeYears * data.count;
    countTogo -= wholeYears * data.count;

    // Check the last year in the recurrence.
    for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
      if (static_cast<uint>(data.daysPerMonth) >= countTogo) {
        // Check the last month in the recurrence
        data.month = *im.current();
        days = data.dayList();
        for (id = days->begin();  id != days->end();  ++id) {
          ++countGone;
          if (--countTogo == 0) {
            data.day = *id;
            goto ex;
          }
        }
      }
      countTogo -= data.daysPerMonth;
      countGone += data.daysPerMonth;
    }
    data.year += rFreq;
  }
ex:
  enddate = data.date();
  return countGone;
}

int Recurrence::yearlyPosCalcToDate(const QDate &enddate, YearlyPosData &data) const
{
  int countGone = 0;
  int countMax  = (rDuration > 0) ? rDuration : INT_MAX;
  int endYear  = enddate.year();
  int endMonth = enddate.month();
  int endDay   = enddate.day();
  if (endDay < data.day && --endMonth == 0) {
    endMonth = 12;
    --endYear;
  }
  int endYearMonth = endYear*12 + endMonth;
  QValueList<int>::ConstIterator id;
  const QValueList<int>* days;

  if (data.month > 1 || data.day > 1) {
    // Check what remains of the start year
    for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
      if (*im.current() >= data.month) {
        data.month = *im.current();
        if (data.yearMonth() > endYearMonth)
          return countGone;
        // Check what remains of the start month
        bool lastMonth = (data.yearMonth() == endYearMonth);
        if (lastMonth || data.day > 1 || data.varies) {
          days = data.dayList();
          if (lastMonth || data.day > 1) {
            for (id = days->begin();  id != days->end();  ++id) {
              if (*id >= data.day) {
                if (lastMonth && *id > endDay)
                  return countGone;
                if (++countGone >= countMax)
                  return countMax;
              }
            }
          } else {
            countGone += days->count();
            if (countGone >= countMax)
              return countMax;
          }
          data.day = 1;
        } else {
          // The number of days per month is constant, so skip
          // the whole month.
          countGone += data.daysPerMonth;
          if (countGone >= countMax)
            return countMax;
        }
      }
    }
    data.month = 1;
    data.year += rFreq;
  }

  if (data.varies) {
    // The number of recurrences varies from year to year.
    for ( ; ; ) {
      for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
        data.month = *im.current();
        days = data.dayList();
        if (data.yearMonth() >= endYearMonth) {
          if (data.yearMonth() > endYearMonth)
            return countGone;
          // Check the last month in the recurrence
          for (id = days->begin();  id != days->end();  ++id) {
            if (*id > endDay)
              return countGone;
            if (++countGone >= countMax)
              return countMax;
          }
        } else {
          countGone += days->count();
          if (countGone >= countMax)
            return countMax;
        }
      }
      data.year += rFreq;
    }
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years, but leave at least
    // 1 recurrence remaining, in order to get its date.
    int wholeYears = endYear - data.year;
    countGone += (wholeYears / rFreq) * data.count;
    if (countGone >= countMax)
      return countMax;
    if (wholeYears % rFreq)
      return countGone;      // end year isn't a recurrence year
    data.year = endYear;

    // Check the last year in the recurrence.
    for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
      data.month = *im.current();
      if (data.month >= endMonth) {
        if (data.month > endMonth)
          return countGone;
        // Check the last month in the recurrence
        days = data.dayList();
        for (id = days->begin();  id != days->end();  ++id) {
          if (*id > endDay)
            return countGone;
          if (++countGone >= countMax)
            return countMax;
        }
      } else {
        countGone += data.daysPerMonth;
        if (countGone >= countMax)
          return countMax;
      }
    }
  }
  return countGone;
}

int Recurrence::yearlyPosCalcNextAfter(QDate &enddate, YearlyPosData &data) const
{
  uint countTogo = (rDuration > 0) ? rDuration : UINT_MAX;
  int  countGone = 0;
  int endYear  = enddate.year();
  int endMonth = enddate.month();
  int endDay   = enddate.day();
  if (endDay < data.day && --endMonth == 0) {
    endMonth = 12;
    --endYear;
  }
  int endYearMonth = endYear*12 + endMonth;
  QValueList<int>::ConstIterator id;
  const QValueList<int>* days;

  if (data.varies) {
    // The number of recurrences varies from year to year.
    for ( ; ; ) {
      // Check the next year
      for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
        if (*im.current() >= data.month) {
          // Check the next month
          data.month = *im.current();
          int ended = data.yearMonth() - endYearMonth;
          days = data.dayList();
          if (ended >= 0 || data.day > 1) {
            // This is the start or end month, so check each day
            for (id = days->begin();  id != days->end();  ++id) {
              if (*id >= data.day) {
                ++countGone;
                if (ended > 0 || (ended == 0 && *id > endDay)) {
                  data.day = *id;
                  goto ex;
                }
                if (--countTogo == 0)
                  return 0;
              }
            }
          } else {
            // Skip the whole month
            uint n = days->count();
            if (n >= countTogo)
              return 0;
            countGone += n;
          }
          data.day = 1;      // we've checked the start month now
        }
      }
      data.month = 1;        // we've checked the start year now
      data.year += rFreq;
    }
  } else {
    // The number of recurrences is the same every year.
    if (data.month > 1 || data.day > 1) {
      // Check what remains of the start year
      for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
        if (*im.current() >= data.month) {
          // Check what remains of the start month
          data.month = *im.current();
          int ended = data.yearMonth() - endYearMonth;
          if (ended >= 0 || data.day > 1) {
            // This is the start or end month, so check each day
            days = data.dayList();
            for (id = days->begin();  id != days->end();  ++id) {
              if (*id >= data.day) {
                ++countGone;
                if (ended > 0 || (ended == 0 && *id > endDay)) {
                  data.day = *id;
                  goto ex;
                }
                if (--countTogo == 0)
                  return 0;
              }
            }
            data.day = 1;      // we've checked the start month now
          } else {
            // Skip the whole month.
            if (static_cast<uint>(data.daysPerMonth) >= countTogo)
              return 0;
            countGone += data.daysPerMonth;
          }
        }
      }
      data.year += rFreq;
    }
    // Skip the remaining whole years to at least endYear.
    int recurYears = (endYear - data.year + rFreq - 1) / rFreq;
    if ((endYear - data.year)%rFreq == 0
    &&  *rYearNums.getLast() <= endMonth)
      ++recurYears;    // required year is after endYear
    if (recurYears) {
      int n = recurYears * data.count;
      if (static_cast<uint>(n) > countTogo)
        return 0;     // reached end of recurrence
      countTogo -= n;
      countGone += n;
      data.year += recurYears * rFreq;
    }

    // Check the last year in the recurrence
    for (QPtrListIterator<int> im(rYearNums); im.current(); ++im) {
      data.month = *im.current();
      int ended = data.yearMonth() - endYearMonth;
      if (ended >= 0) {
        // This is the end month, so check each day
        days = data.dayList();
        for (id = days->begin();  id != days->end();  ++id) {
          ++countGone;
          if (ended > 0 || (ended == 0 && *id > endDay)) {
            data.day = *id;
            goto ex;
          }
          if (--countTogo == 0)
            return 0;
        }
      } else {
        // Skip the whole month.
        if (static_cast<uint>(data.daysPerMonth) >= countTogo)
          return 0;
        countGone += data.daysPerMonth;
      }
    }
  }
ex:
  enddate = data.date();
  return countGone;
}


/* Find count and, depending on 'func', the end date of an annual recurrence by day.
 * Reply = total number of occurrences up to 'enddate', or 0 if error.
 * If 'func' = END_DATE_AND_COUNT or NEXT_AFTER_DATE, 'enddate' is updated to the
 * recurrence end date.
 */
class Recurrence::YearlyDayData
{
  public:
    int    year;       // current year
    int    day;        // current day of year 1..366
    bool   varies;     // true if day 366 recurs

  private:
    int    daycount;

  public:
    YearlyDayData(const Recurrence* r, const QDate &date)
      : year( date.year() ), day( date.dayOfYear() ),
        varies( *r->yearNums().getLast() == 366 ),
        daycount( r->yearNums().count() ) { }
    bool  leapYear() const       { return QDate::leapYear(year); }
    int   dayCount() const       { return daycount - (varies && !QDate::leapYear(year) ? 1 : 0); }
    bool  isMaxDayCount() const  { return !varies || QDate::leapYear(year); }
    QDate date() const           { return QDate(year, 1, 1).addDays(day - 1); }
};

int Recurrence::yearlyDayCalc(PeriodFunc func, QDate &enddate) const
{
  if (rYearNums.isEmpty())
    return 0;
  YearlyDayData data(this, mRecurStart.date());
  switch (func) {
    case END_DATE_AND_COUNT:
      return yearlyDayCalcEndDate(enddate, data);
    case COUNT_TO_DATE:
      return yearlyDayCalcToDate(enddate, data);
    case NEXT_AFTER_DATE:
      return yearlyDayCalcNextAfter(enddate, data);
  }
  return 0;
}

int Recurrence::yearlyDayCalcEndDate(QDate &enddate, YearlyDayData &data) const
{
  uint countTogo = rDuration;
  int countGone = 0;

  if (data.day > 1) {
    // Check what remains of the start year
    bool leapOK = data.isMaxDayCount();
    for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
      int d = *it.current();
      if (d >= data.day && (leapOK || d < 366)) {
        ++countGone;
        if (--countTogo == 0) {
          data.day = d;
          goto ex;
        }
      }
    }
    data.day = 1;
    data.year += rFreq;
  }

  if (data.varies) {
    // The number of recurrences is different in leap years,
    // so check year-by-year.
    for ( ; ; ) {
      uint n = data.dayCount();
      if (n >= countTogo)
        break;
      countTogo -= n;
      countGone += n;
      data.year += rFreq;
    }
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years, but leave at least
    // 1 recurrence remaining, in order to get its date.
    int daysPerYear = rYearNums.count();
    int wholeYears = (countTogo - 1) / daysPerYear;
    data.year += wholeYears * rFreq;
    countGone += wholeYears * daysPerYear;
    countTogo -= wholeYears * daysPerYear;
  }
  if (countTogo) {
    // Check the last year in the recurrence
    for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
      ++countGone;
      if (--countTogo == 0) {
        data.day = *it.current();
        break;
      }
    }
  }
ex:
  enddate = data.date();
  return countGone;
}

int Recurrence::yearlyDayCalcToDate(const QDate &enddate, YearlyDayData &data) const
{
  int countGone = 0;
  int countMax  = (rDuration > 0) ? rDuration : INT_MAX;
  int endYear = enddate.year();
  int endDay  = enddate.dayOfYear();

  if (data.day > 1) {
    // Check what remains of the start year
    bool leapOK = data.isMaxDayCount();
    for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
      int d = *it.current();
      if (d >= data.day && (leapOK || d < 366)) {
        if (data.year == endYear && d > endDay)
          return countGone;
        if (++countGone >= countMax)
          return countMax;
      }
    }
    data.day = 1;
    data.year += rFreq;
  }

  if (data.varies) {
    // The number of recurrences is different in leap years,
    // so check year-by-year.
    while (data.year < endYear) {
      uint n = data.dayCount();
      countGone += n;
      if (countGone >= countMax)
        return countMax;
      data.year += rFreq;
    }
    if (data.year > endYear)
      return countGone;
  } else {
    // The number of recurrences is the same every year.
    // Skip the remaining whole years.
    int wholeYears = endYear - data.year;
    countGone += (wholeYears / rFreq) * rYearNums.count();
    if (countGone >= countMax)
      return countMax;
    if (wholeYears % rFreq)
      return countGone;      // end year isn't a recurrence year
    data.year = endYear;
  }

  if (data.year <= endYear) {
    // Check the last year in the recurrence
    for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
      if (*it.current() > endDay)
        return countGone;
      if (++countGone >= countMax)
        return countMax;
    }
  }
  return countGone;
}

int Recurrence::yearlyDayCalcNextAfter(QDate &enddate, YearlyDayData &data) const
{
  uint countTogo = (rDuration > 0) ? rDuration : UINT_MAX;
  int  countGone = 0;
  int  endYear = enddate.year();
  int  endDay  = enddate.dayOfYear();

  if (data.day > 1) {
    // Check what remains of the start year
    bool leapOK = data.isMaxDayCount();
    for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
      int d = *it.current();
      if (d >= data.day && (leapOK || d < 366)) {
        ++countGone;
        if (data.year == endYear && d > endDay) {
          data.day = d;
          goto ex;
        }
        if (--countTogo == 0)
          return 0;
      }
    }
    data.day = 1;
    data.year += rFreq;
  }

  if (data.varies) {
    // The number of recurrences is different in leap years,
    // so check year-by-year.
    while (data.year <= endYear) {
      uint n = data.dayCount();
      if (data.year == endYear && *rYearNums.getLast() > endDay)
        break;
      if (n >= countTogo)
        break;
      countTogo -= n;
      countGone += n;
      data.year += rFreq;
    }
  } else {
    // The number of recurrences is the same every year,
    // so skip the year-by-year check.
    // Skip the remaining whole years to at least endYear.
    int daysPerYear = rYearNums.count();
    int recurYears = (endYear - data.year + rFreq - 1) / rFreq;
    if ((endYear - data.year)%rFreq == 0
    &&  *rYearNums.getLast() <= endDay)
      ++recurYears;    // required year is after endYear
    if (recurYears) {
      int n = recurYears * daysPerYear;
      if (static_cast<uint>(n) > countTogo)
        return 0;     // reached end of recurrence
      countTogo -= n;
      countGone += n;
      data.year += recurYears * rFreq;
    }
  }

  // Check the last year in the recurrence
  for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
    ++countGone;
    int d = *it.current();
    if (data.year > endYear || d > endDay) {
      data.day = d;
      break;
    }
    if (--countTogo == 0)
      return 0;
  }
ex:
  enddate = data.date();
  return countGone;
}

// Get the days in this month which recur, in numerical order.
// Parameters: daysInMonth = number of days in this month
//             startDayOfWeek = day of week for first day of month.
void Recurrence::getMonthlyPosDays(QValueList<int> &list, int daysInMonth, int startDayOfWeek) const
{
  list.clear();
  int endDayOfWeek = (startDayOfWeek + daysInMonth - 2) % 7 + 1;
  // Go through the list, compiling a bit list of actual day numbers
  Q_UINT32 days = 0;
  for (QPtrListIterator<rMonthPos> pos(rMonthPositions); pos.current(); ++pos) {
    int weeknum = pos.current()->rPos - 1;   // get 0-based week number
    QBitArray &rdays = pos.current()->rDays;
    if (pos.current()->negative) {
      // nth days before the end of the month
      for (uint i = 1; i <= 7; ++i) {
        if (rdays.testBit(i - 1)) {
          int day = daysInMonth - weeknum*7 - (endDayOfWeek - i + 7) % 7;
          if (day > 0)
            days |= 1 << (day - 1);
        }
      }
    } else {
      // nth days after the start of the month
      for (uint i = 1; i <= 7; ++i) {
        if (rdays.testBit(i - 1)) {
          int day = 1 + weeknum*7 + (i - startDayOfWeek + 7) % 7;
          if (day <= daysInMonth)
            days |= 1 << (day - 1);
        }
      }
    }
  }
  // Compile the ordered list
  Q_UINT32 mask = 1;
  for (int i = 0; i < daysInMonth; mask <<= 1, ++i) {
    if (days & mask)
      list.append(i + 1);
  }
}

// Get the number of days in the month which recur.
// Reply = -1 if the number varies from month to month.
int Recurrence::countMonthlyPosDays() const
{
  int count = 0;
  Q_UINT8 positive[5] = { 0, 0, 0, 0, 0 };
  Q_UINT8 negative[4] = { 0, 0, 0, 0 };
  for (QPtrListIterator<rMonthPos> pos(rMonthPositions); pos.current(); ++pos) {
    int weeknum = pos.current()->rPos;
    Q_UINT8* wk;
    if (pos.current()->negative) {
      // nth days before the end of the month
      if (weeknum > 4)
        return -1;       // days in 5th week are often missing
      wk = &negative[4 - weeknum];
    } else {
      // nth days after the start of the month
      if (weeknum > 4)
        return -1;       // days in 5th week are often missing
      wk = &positive[weeknum - 1];
    }
    QBitArray &rdays = pos.current()->rDays;
    for (uint i = 0; i < 7; ++i) {
      if (rdays.testBit(i)) {
        ++count;
        *wk |= (1 << i);
      }
    }
  }
  // Check for any possible days which could be duplicated by
  // a positive and a negative position.
  for (int i = 0; i < 4; ++i) {
    if (negative[i] & (positive[i] | positive[i+1]))
      return -1;
  }
  return count;
}

// Get the days in this month which recur, in numerical order.
// Reply = true if day numbers varies from month to month.
bool Recurrence::getMonthlyDayDays(QValueList<int> &list, int daysInMonth) const
{
  list.clear();
  bool variable = false;
  Q_UINT32 days = 0;
  for (QPtrListIterator<int> it(rMonthDays); it.current(); ++it) {
    int day = *it.current();
    if (day > 0) {
      // date in the month
      if (day <= daysInMonth)
        days |= 1 << (day - 1);
      if (day > 28 && day <= 31)
        variable = true;     // this date does not appear in some months
    } else if (day < 0) {
      // days before the end of the month
      variable = true;       // this date varies depending on the month length
      day = daysInMonth + day;    // zero-based day of month
      if (day >= 0)
        days |= 1 << day;
    }
  }
  // Compile the ordered list
  Q_UINT32 mask = 1;
  for (int i = 0; i < daysInMonth; mask <<= 1, ++i) {
    if (days & mask)
      list.append(i + 1);
  }
  return variable;
}

// Get the months which recur, in numerical order, for both leap years and non-leap years.
// N.B. If February 29th recurs on March 1st in non-leap years, February (not March) is
// included in the non-leap year month list.
// Reply = true if February 29th also recurs.
bool Recurrence::getYearlyMonthMonths(int day, QValueList<int> &list, QValueList<int> &leaplist) const
{
  list.clear();
  leaplist.clear();
  bool feb29 = false;
  for (QPtrListIterator<int> it(rYearNums); it.current(); ++it) {
    int month = *it.current();
    if (month == 2) {
      if (day <= 28) {
        list.append(month);     // date appears in February
        leaplist.append(month);
      }
      else if (day == 29) {
        // February 29th
        leaplist.append(month);
        switch (mFeb29YearlyType) {
          case rFeb28:
          case rMar1:
            list.append(2);
            break;
          case rFeb29:
            break;
        }
        feb29 = true;
      }
    }
    else if (day <= 30 || QDate(2000, month, 1).daysInMonth() == 31) {
      list.append(month);       // date appears in every month
      leaplist.append(month);
    }
  }
  return feb29;
}

/* From the recurrence day of the week list, get the earliest day in the
 * specified week which is >= the startDay.
 * Parameters:  startDay = 1..7 (Monday..Sunday)
 *              useWeekStart = true to end search at day before next rWeekStart
 *                           = false to search for a full 7 days
 * Reply = day of the week (1..7), or 0 if none found.
 */
int Recurrence::getFirstDayInWeek(int startDay, bool useWeekStart) const
{
  int last = ((useWeekStart ? rWeekStart : startDay) + 5)%7;
  for (int i = startDay - 1;  ;  i = (i + 1)%7) {
    if (rDays.testBit(i))
      return i + 1;
    if (i == last)
      return 0;
  }
}

/* From the recurrence day of the week list, get the latest day in the
 * specified week which is <= the endDay.
 * Parameters:  endDay = 1..7 (Monday..Sunday)
 *              useWeekStart = true to end search at rWeekStart
 *                           = false to search for a full 7 days
 * Reply = day of the week (1..7), or 0 if none found.
 */
int Recurrence::getLastDayInWeek(int endDay, bool useWeekStart) const
{
  int last = useWeekStart ? rWeekStart - 1 : endDay%7;
  for (int i = endDay - 1;  ;  i = (i + 6)%7) {
    if (rDays.testBit(i))
      return i + 1;
    if (i == last)
      return 0;
  }
}

/* From the recurrence monthly day number list or monthly day of week/week of
 * month list, get the earliest day in the specified month which is >= the
 * earliestDate.
 */
QDate Recurrence::getFirstDateInMonth(const QDate &earliestDate) const
{
  int earliestDay = earliestDate.day();
  int daysInMonth = earliestDate.daysInMonth();
  switch (recurs) {
    case rMonthlyDay: {
      int minday = daysInMonth + 1;
      for (QPtrListIterator<int> it(rMonthDays);  it.current();  ++it) {
        int day = *it.current();
        if (day < 0)
          day = daysInMonth + day + 1;
        if (day >= earliestDay  &&  day < minday)
          minday = day;
      }
      if (minday <= daysInMonth)
        return earliestDate.addDays(minday - earliestDay);
      break;
    }
    case rMonthlyPos:
    case rYearlyPos: {
      QDate monthBegin(earliestDate.addDays(1 - earliestDay));
      QValueList<int> dayList;
      getMonthlyPosDays(dayList, daysInMonth, monthBegin.dayOfWeek());
      for (QValueList<int>::ConstIterator id = dayList.begin();  id != dayList.end();  ++id) {
        if (*id >= earliestDay)
          return monthBegin.addDays(*id - 1);
      }
      break;
    }
  }
  return QDate();
}

/* From the recurrence monthly day number list or monthly day of week/week of
 * month list, get the latest day in the specified month which is <= the
 * latestDate.
 */
QDate Recurrence::getLastDateInMonth(const QDate &latestDate) const
{
  int latestDay = latestDate.day();
  int daysInMonth = latestDate.daysInMonth();
  switch (recurs) {
    case rMonthlyDay: {
      int maxday = -1;
      for (QPtrListIterator<int> it(rMonthDays);  it.current();  ++it) {
        int day = *it.current();
        if (day < 0)
          day = daysInMonth + day + 1;
        if (day <= latestDay  &&  day > maxday)
          maxday = day;
      }
      if (maxday > 0)
        return QDate(latestDate.year(), latestDate.month(), maxday);
      break;
    }
    case rMonthlyPos:
    case rYearlyPos: {
      QDate monthBegin(latestDate.addDays(1 - latestDay));
      QValueList<int> dayList;
      getMonthlyPosDays(dayList, daysInMonth, monthBegin.dayOfWeek());
      for (QValueList<int>::ConstIterator id = dayList.fromLast();  id != dayList.end();  --id) {
        if (*id <= latestDay)
          return monthBegin.addDays(*id - 1);
      }
      break;
    }
  }
  return QDate();
}

/* From the recurrence yearly month list or yearly day list, get the earliest
 * month or day in the specified year which is >= the earliestDate.
 * Note that rYearNums is sorted in numerical order.
 */
QDate Recurrence::getFirstDateInYear(const QDate &earliestDate) const
{
  QPtrListIterator<int> it(rYearNums);
  switch (recurs) {
    case rYearlyMonth: {
      int earliestYear  = earliestDate.year();
      int earliestMonth = earliestDate.month();
      int earliestDay   = earliestDate.day();
      int day = rMonthDays.count() ? *rMonthDays.getFirst() : recurStart().date().day();
      int dayThisMonth = (day > 0) ? day : earliestDate.daysInMonth() + 1 + day;
      if (earliestDay > dayThisMonth) {
        // The earliest date is later in the month than the recurrence date,
        // so skip to the next month before starting to check
        if (++earliestMonth > 12)
          return QDate();
      }
      for ( ;  it.current();  ++it) {
        int month = *it.current();
        if (month >= earliestMonth) {
          dayThisMonth = (day > 0) ? day : QDate(earliestYear, month, 1).daysInMonth() + 1 + day;
          if (dayThisMonth <= 28  ||  QDate::isValid(earliestYear, month, dayThisMonth))
            return QDate(earliestYear, month, dayThisMonth);
          if (day == 29  &&  month == 2) {
            // It's a recurrence on February 29th, in a non-leap year
            switch (mFeb29YearlyType) {
              case rMar1:
                return QDate(earliestYear, 3, 1);
              case rFeb28:
                if (earliestDay <= 28)
                  return QDate(earliestYear, 2, 28);
                break;
              case rFeb29:
                break;
            }
          }
        }
      }
      break;
    }
    case rYearlyPos: {
      QValueList<int> dayList;
      int earliestYear  = earliestDate.year();
      int earliestMonth = earliestDate.month();
      int earliestDay   = earliestDate.day();
      for ( ;  it.current();  ++it) {
        int month = *it.current();
        if (month >= earliestMonth) {
          QDate monthBegin(earliestYear, month, 1);
          getMonthlyPosDays(dayList, monthBegin.daysInMonth(), monthBegin.dayOfWeek());
          for (QValueList<int>::ConstIterator id = dayList.begin();  id != dayList.end();  ++id) {
            if (*id >= earliestDay)
              return monthBegin.addDays(*id - 1);
          }
          earliestDay = 1;
        }
      }
      break;
    }
    case rYearlyDay: {
      int earliestDay = earliestDate.dayOfYear();
      for ( ;  it.current();  ++it) {
        int day = *it.current();
        if (day >= earliestDay && (day <= 365 || day <= earliestDate.daysInYear()))
          return earliestDate.addDays(day - earliestDay);
      }
      break;
    }
  }
  return QDate();
}

/* From the recurrence yearly month list or yearly day list, get the latest
 * month or day in the specified year which is <= the latestDate.
 * Note that rYearNums is sorted in numerical order.
 */
QDate Recurrence::getLastDateInYear(const QDate &latestDate) const
{
  QPtrListIterator<int> it(rYearNums);
  switch (recurs) {
    case rYearlyMonth: {
      int latestYear  = latestDate.year();
      int latestMonth = latestDate.month();
      int day = rMonthDays.count() ? *rMonthDays.getFirst() : recurStart().date().day();
      int dayThisMonth = (day > 0) ? day : latestDate.daysInMonth() + 1 + day;
      if (latestDate.day() < dayThisMonth) {
        // The latest date is earlier in the month than the recurrence date,
        // so skip to the previous month before starting to check
        if (--latestMonth <= 0)
          return QDate();
      }
      for (it.toLast();  it.current();  --it) {
        int month = *it.current();
        if (month <= latestMonth) {
          dayThisMonth = (day > 0) ? day : QDate(latestYear, month, 1).daysInMonth() + 1 + day;
          if (dayThisMonth <= 28  ||  QDate::isValid(latestYear, month, dayThisMonth))
            return QDate(latestYear, month, dayThisMonth);
          if (day == 29  &&  month == 2) {
            // It's a recurrence on February 29th, in a non-leap year
            switch (mFeb29YearlyType) {
              case rMar1:
                if (latestMonth >= 3)
                  return QDate(latestYear, 3, 1);
                break;
              case rFeb28:
                return QDate(latestYear, 2, 28);
              case rFeb29:
                break;
            }
          }
        }
      }
      break;
    }
    case rYearlyPos: {
      QValueList<int> dayList;
      int latestYear  = latestDate.year();
      int latestMonth = latestDate.month();
      int latestDay   = latestDate.day();
      for (it.toLast();  it.current();  --it) {
        int month = *it.current();
        if (month <= latestMonth) {
          QDate monthBegin(latestYear, month, 1);
          getMonthlyPosDays(dayList, monthBegin.daysInMonth(), monthBegin.dayOfWeek());
          for (QValueList<int>::ConstIterator id = dayList.fromLast();  id != dayList.end();  --id) {
            if (*id <= latestDay)
              return monthBegin.addDays(*id - 1);
          }
          latestDay = 31;
        }
      }
      break;
    }
    case rYearlyDay: {
      int latestDay = latestDate.dayOfYear();
      for (it.toLast();  it.current();  --it) {
        int day = *it.current();
        if (day <= latestDay)
          return latestDate.addDays(day - latestDay);
      }
      break;
    }
  }
  return QDate();
}

void Recurrence::dump() const
{
  kdDebug(5800) << "Recurrence::dump():" << endl;

  kdDebug(5800) << "  type: " << recurs << endl;

  kdDebug(5800) << "  rDays: " << endl;
  int i;
  for( i = 0; i < 7; ++i ) {
    kdDebug(5800) << "    " << i << ": "
              << ( rDays.testBit( i ) ? "true" : "false" ) << endl;
  }
  kdDebug(5800) << "  duration: " << rDuration << endl;

  for (QPtrListIterator<int> it(rMonthDays);  it.current();  ++it) {
    kdDebug(5800) << "  monthday: " << *(it.current()) << endl;
  }
}
