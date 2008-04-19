/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
    Copyright (C) 2005 Reinhold Kainhofer <kainhofer@kde.org>

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

#include <limits.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <qbitarray.h>

#include "recurrence.h"
#include "recurrencerule.h"

using namespace KCal;


Recurrence::Recurrence()
: mFloating( false ),
  mRecurReadOnly(false),
  mCachedType(rMax)
{
  mExRules.setAutoDelete( true );
  mRRules.setAutoDelete( true );
}

Recurrence::Recurrence( const Recurrence &r )
: RecurrenceRule::Observer(),
  mRDateTimes( r.mRDateTimes ), mRDates( r.mRDates ),
  mExDateTimes( r.mExDateTimes ), mExDates( r.mExDates ),
  mStartDateTime( r.mStartDateTime ),
  mFloating( r.mFloating ),
  mRecurReadOnly(r.mRecurReadOnly),
  mCachedType( r.mCachedType )
{
  mExRules.setAutoDelete( true );
  mRRules.setAutoDelete( true );
  RecurrenceRule::List::ConstIterator rr;
  for ( rr = r.mRRules.begin(); rr != r.mRRules.end(); ++rr ) {
    RecurrenceRule *rule = new RecurrenceRule( *(*rr) );
    mRRules.append( rule );
    rule->addObserver( this );
  }
  for ( rr = r.mExRules.begin(); rr != r.mExRules.end(); ++rr ) {
    RecurrenceRule *rule = new RecurrenceRule( *(*rr) );
    mExRules.append( rule );
    rule->addObserver( this );
  }
}

Recurrence::~Recurrence()
{
}



bool Recurrence::operator==( const Recurrence& r2 ) const
{
  if ( mStartDateTime != r2.mStartDateTime
  ||   mFloating != r2.mFloating
  ||   mRecurReadOnly != r2.mRecurReadOnly )
    return false;
  if ( mExDates != r2.mExDates ) return false;
  if ( mExDateTimes != r2.mExDateTimes ) return false;
  if ( mRDates != r2.mRDates ) return false;
  if ( mRDateTimes != r2.mRDateTimes ) return false;

// Compare the rrules, exrules! Assume they have the same order... This only
// matters if we have more than one rule (which shouldn't be the default anyway)
  if ( mRRules.count() != r2.mRRules.count() ) return false;
  RecurrenceRule::List::ConstIterator rit1 = mRRules.begin();
  RecurrenceRule::List::ConstIterator rit2 = r2.mRRules.begin();

  while ( rit1 != mRRules.end() && rit2 != r2.mRRules.end() ) {
    // dereference the iterator to the RecurrenceRule*, and that once again
    // to RecurrenceRule...
    if ( *(*rit1) != *(*rit2) ) return false;
    ++rit1;
    ++rit2;
  }
  RecurrenceRule::List::ConstIterator exit1 = mExRules.begin();
  RecurrenceRule::List::ConstIterator exit2 = r2.mExRules.begin();

  while ( exit1 != mExRules.end() && exit2 != r2.mExRules.end() ) {
    // dereference the iterator to the RecurrenceRule*, and that once again
    // to RecurrenceRule...
    if ( *(*exit1) != *(*exit2) ) return false;
    ++exit1;
    ++exit2;
  }
  return true;
}

void Recurrence::addObserver( Observer *observer )
{
  if ( !mObservers.contains( observer ) )
    mObservers.append( observer );
}

void Recurrence::removeObserver( Observer *observer )
{
  if ( mObservers.contains( observer ) )
    mObservers.remove( observer );
}


QDateTime Recurrence::startDateTime() const
{
  if ( mFloating )
    return QDateTime( mStartDateTime.date(), QTime( 0, 0, 0 ) );
  else return mStartDateTime;
}

void Recurrence::setFloats( bool floats )
{
  if ( mRecurReadOnly ) return;
  if ( floats == mFloating ) return;
  mFloating = floats;


  RecurrenceRule::List::ConstIterator it;
  for ( it = mRRules.begin(); it != mRRules.end(); ++it ) {
    (*it)->setFloats( floats );
  }

  RecurrenceRule::List::ConstIterator it1;
  for ( it1 = mExRules.begin(); it1 != mExRules.end(); ++it1 ) {
    (*it1)->setFloats( floats );
  }
  updated();
}

RecurrenceRule *Recurrence::defaultRRule( bool create ) const
{
  if ( mRRules.isEmpty() ) {
    if ( !create || mRecurReadOnly ) return 0;
    RecurrenceRule *rrule = new RecurrenceRule();
    rrule->setStartDt( startDateTime() );
    const_cast<KCal::Recurrence*>(this)->addRRule( rrule );
    return rrule;
  } else {
    return mRRules.first();
  }
}

RecurrenceRule *Recurrence::defaultRRuleConst() const
{
  if ( mRRules.isEmpty() ) {
    return 0;
  } else {
    return mRRules.first();
  }
}

void Recurrence::updated()
{
  // recurrenceType() re-calculates the type if it's rMax
  mCachedType = rMax;
  for ( QValueList<Observer*>::ConstIterator it = mObservers.begin();
        it != mObservers.end(); ++it ) {
    if ( (*it) ) (*it)->recurrenceUpdated( this );
  }
}

bool Recurrence::doesRecur() const
{
  return !mRRules.isEmpty() || !mRDates.isEmpty() || !mRDateTimes.isEmpty();
}

ushort Recurrence::recurrenceType() const
{
  if ( mCachedType == rMax ) {
    mCachedType = recurrenceType( defaultRRuleConst() );
  }
  return mCachedType;
}

ushort Recurrence::recurrenceType( const RecurrenceRule *rrule )
{
  if ( !rrule ) return rNone;
  RecurrenceRule::PeriodType type = rrule->recurrenceType();

  // BYSETPOS, BYWEEKNUMBER and BYSECOND were not supported in old versions
  if ( !rrule->bySetPos().isEmpty() )
    return rOther;
  if ( !rrule->bySeconds().isEmpty() )
    return rOther;
  if ( !rrule->byWeekNumbers().isEmpty() )
    return rOther;

  // It wasn't possible to set BYMINUTES, BYHOUR etc. by the old code. So if
  // it's set, it's none of the old types
  if ( !rrule->byMinutes().isEmpty() )
    return rOther;
  if ( !rrule->byHours().isEmpty() )
    return rOther;

  // Possible combinations were:
  // BYDAY: with WEEKLY, MONTHLY, YEARLY
  // BYMONTHDAY: with MONTHLY, YEARLY
  // BYMONTH: with YEARLY
  // BYYEARDAY: with YEARLY
  if ( !rrule->byYearDays().isEmpty() && type != RecurrenceRule::rYearly )
    return rOther;
  if ( !rrule->byMonths().isEmpty() && type != RecurrenceRule::rYearly )
    return rOther;
  if ( !rrule->byDays().isEmpty() ) {
    if ( type != RecurrenceRule::rYearly && type != RecurrenceRule::rMonthly &&
         type != RecurrenceRule::rWeekly )
      return rOther;
  }

  switch ( type ) {
    case RecurrenceRule::rNone:     return rNone;
    case RecurrenceRule::rMinutely: return rMinutely;
    case RecurrenceRule::rHourly:   return rHourly;
    case RecurrenceRule::rDaily:    return rDaily;
    case RecurrenceRule::rWeekly:   return rWeekly;
    case RecurrenceRule::rMonthly: {
        if ( rrule->byDays().isEmpty() ) return rMonthlyDay;
        else if ( rrule->byMonthDays().isEmpty() ) return rMonthlyPos;
        else return rOther; // both position and date specified
      }
    case RecurrenceRule::rYearly: {
        // Possible combinations:
        //   rYearlyMonth: [BYMONTH &] BYMONTHDAY
        //   rYearlyDay: BYYEARDAY
        //   rYearlyPos: [BYMONTH &] BYDAY
        if ( !rrule->byDays().isEmpty() ) {
          // can only by rYearlyPos
          if ( rrule->byMonthDays().isEmpty() && rrule->byYearDays().isEmpty() )
            return rYearlyPos;
          else return rOther;
        } else if ( !rrule->byYearDays().isEmpty() ) {
          // Can only be rYearlyDay
          if ( rrule->byMonths().isEmpty() && rrule->byMonthDays().isEmpty() )
            return rYearlyDay;
          else return rOther;
        } else {
          return rYearlyMonth;
        }
        break;
      }
     default: return rOther;
  }
  return rOther;
}

bool Recurrence::recursOn(const QDate &qd) const
{
  TimeList tms;
  // First handle dates. Exrules override
  if ( mExDates.contains( qd ) ) return false;
  // For all-day events a matching exrule excludes the whole day
  // since exclusions take precedence over inclusions, we know it can't occur on that day.
  if ( doesFloat() ) {
    for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
      if ( (*rr)->recursOn( qd ) )
        return false;
    }
  }

  if ( mRDates.contains( qd ) ) return true;

  // Check if it might recur today at all.
  bool recurs = false;
  if ( startDate() == qd ) recurs = true;
  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    recurs = recurs || (*rr)->recursOn( qd );
  }
  // If we already know it recurs, no need to check the rdate list too.
  if ( !recurs ) {
    for ( DateTimeList::ConstIterator rit = mRDateTimes.begin();
          rit != mRDateTimes.end(); ++rit ) {
      if ( (*rit).date() == qd ) {
        recurs = true;
        break;
      }
    }
  }
  // If the event wouldn't recur at all, simply return false, don't check ex*
  if ( !recurs ) return false;

  // Check if there are any times for this day excluded, either by exdate or exrule:
  bool exon = false;
  for ( DateTimeList::ConstIterator exit = mExDateTimes.begin();
        exit != mExDateTimes.end(); ++exit ) {
    if ( (*exit).date() == qd ) {
      exon = true;
      break;
    }
  }
  if ( !doesFloat() ) {     // we have already checked floating times above
    for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
      exon = exon || (*rr)->recursOn( qd );
    }
  }

  if ( !exon ) {
    // Simple case, nothing on that day excluded, return the value from before
    return recurs;
  } else {
    // Harder part: I don't think there is any way other than to calculate the
    // whole list of items for that day.
    TimeList timesForDay( recurTimesOn( qd ) );
    return !timesForDay.isEmpty();
  }
}

bool Recurrence::recursAt( const QDateTime &dt ) const
{
  // if it's excluded anyway, don't bother to check if it recurs at all.
  if ( mExDateTimes.contains( dt )) return false;
  if ( mExDates.contains( dt.date() )) return false;
  for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
    if ( (*rr)->recursAt( dt ) ) return false;
  }

  // Check explicit recurrences, then rrules.
  bool occurs = ( startDateTime() == dt ) || mRDateTimes.contains( dt );
  if ( occurs )
    return true;
  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    if ( (*rr)->recursAt( dt ) ) return true;
  }

  return false;
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
QDateTime Recurrence::endDateTime() const
{
  DateTimeList dts;
  dts << startDateTime();
  if ( !mRDates.isEmpty() ) dts << QDateTime( mRDates.last(), QTime( 0, 0, 0 ) );
  if ( !mRDateTimes.isEmpty() ) dts << mRDateTimes.last();
  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    QDateTime rl( (*rr)->endDt() );
    // if any of the rules is infinite, the whole recurrence is
    if ( !rl.isValid() ) return QDateTime();
    dts << rl;
  }
  qSortUnique( dts );
  if ( dts.isEmpty() ) return QDateTime();
  else return dts.last();
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
QDate Recurrence::endDate() const
{
  QDateTime end( endDateTime() );
  if ( end.isValid() ) { return end.date(); }
  else return QDate();
}

void Recurrence::setEndDate( const QDate &date )
{
  if ( doesFloat() )
    setEndDateTime( QDateTime( date, QTime( 23, 59, 59 ) ) );
  else
    setEndDateTime( QDateTime( date, mStartDateTime.time() ) );
}

void Recurrence::setEndDateTime( const QDateTime &dateTime )
{
  if ( mRecurReadOnly ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;
  rrule->setEndDt( dateTime );
  updated();
}

int Recurrence::duration() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->duration();
  else return 0;
}

// int Recurrence::durationTo( const QDate &/*date*/ ) const
// {
//   return 0;
// }

int Recurrence::durationTo( const QDateTime &datetime ) const
{
  // Emulate old behavior: This is just an interface to the first rule!
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( !rrule ) return 0;
  else return rrule->durationTo( datetime );
}

void Recurrence::setDuration( int duration )
{
  if ( mRecurReadOnly ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;
  rrule->setDuration( duration );
  updated();
}

void Recurrence::unsetRecurs()
{
  if ( mRecurReadOnly ) return;
  mRRules.clearAll();
  updated();
}

void Recurrence::clear()
{
  if ( mRecurReadOnly ) return;
  mRRules.clearAll();
  mExRules.clearAll();
  mRDates.clear();
  mRDateTimes.clear();
  mExDates.clear();
  mExDateTimes.clear();
  mCachedType = rMax;
  updated();
}

void Recurrence::setStartDateTime( const QDateTime &start )
{
  if ( mRecurReadOnly ) return;
  mStartDateTime = start;
  setFloats( false );   // set all RRULEs and EXRULEs

  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    (*rr)->setStartDt( start );
  }
  for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
    (*rr)->setStartDt( start );
  }
  updated();
}

void Recurrence::setStartDate( const QDate &start )
{
  setStartDateTime( QDateTime( start, QTime(0,0,0) ) );
  setFloats( true );
}

int Recurrence::frequency() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->frequency();
  else return 0;
}

// Emulate the old behaviour. Make this methods just an interface to the
// first rrule
void Recurrence::setFrequency( int freq )
{
  if ( mRecurReadOnly || freq <= 0 ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( rrule )
    rrule->setFrequency( freq );
  updated();
}


// WEEKLY

int Recurrence::weekStart() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->weekStart();
  else return 1;
}

// Emulate the old behavior
QBitArray Recurrence::days() const
{
  QBitArray days( 7 );
  days.fill( 0 );
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) {
    QValueList<RecurrenceRule::WDayPos> bydays = rrule->byDays();
    for ( QValueListConstIterator<RecurrenceRule::WDayPos> it = bydays.begin();
          it != bydays.end(); ++it ) {
      if ( (*it).pos() == 0 ) {
        days.setBit( (*it).day() - 1 );
      }
    }
  }
  return days;
}


// MONTHLY

// Emulate the old behavior
QValueList<int> Recurrence::monthDays() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byMonthDays();
  else return QValueList<int>();
}

// Emulate the old behavior
QValueList<RecurrenceRule::WDayPos> Recurrence::monthPositions() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byDays();
  else return QValueList<RecurrenceRule::WDayPos>();
}


// YEARLY

QValueList<int> Recurrence::yearDays() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byYearDays();
  else return QValueList<int>();
}

QValueList<int> Recurrence::yearDates() const
{
  return monthDays();
}

QValueList<int> Recurrence::yearMonths() const
{
  RecurrenceRule *rrule = defaultRRuleConst();
  if ( rrule ) return rrule->byMonths();
  else return QValueList<int>();
}

QValueList<RecurrenceRule::WDayPos> Recurrence::yearPositions() const
{
  return monthPositions();
}



RecurrenceRule *Recurrence::setNewRecurrenceType( RecurrenceRule::PeriodType type, int freq )
{
  if ( mRecurReadOnly || freq <= 0 ) return 0;
  mRRules.clearAll();
  updated();
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return 0;
  rrule->setRecurrenceType( type );
  rrule->setFrequency( freq );
  rrule->setDuration( -1 );
  return rrule;
}

void Recurrence::setMinutely( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rMinutely, _rFreq ) )
    updated();
}

void Recurrence::setHourly( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rHourly, _rFreq ) )
    updated();
}

void Recurrence::setDaily( int _rFreq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rDaily, _rFreq ) )
    updated();
}

void Recurrence::setWeekly( int freq, int weekStart )
{
  RecurrenceRule *rrule = setNewRecurrenceType( RecurrenceRule::rWeekly, freq );
  if ( !rrule ) return;
  rrule->setWeekStart( weekStart );
  updated();
}

void Recurrence::setWeekly( int freq, const QBitArray &days, int weekStart )
{
  setWeekly( freq, weekStart );
  addMonthlyPos( 0, days );
}

void Recurrence::addWeeklyDays( const QBitArray &days )
{
  addMonthlyPos( 0, days );
}

void Recurrence::setMonthly( int freq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rMonthly, freq ) )
    updated();
}

void Recurrence::addMonthlyPos( short pos, const QBitArray &days )
{
  // Allow 53 for yearly!
  if ( mRecurReadOnly || pos > 53 || pos < -53 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;
  bool changed = false;
  QValueList<RecurrenceRule::WDayPos> positions = rrule->byDays();

  for ( int i = 0; i < 7; ++i ) {
    if ( days.testBit(i) ) {
      RecurrenceRule::WDayPos p( pos, i + 1 );
      if ( !positions.contains( p ) ) {
        changed = true;
        positions.append( p );
      }
    }
  }
  if ( changed ) {
    rrule->setByDays( positions );
    updated();
  }
}


void Recurrence::addMonthlyPos( short pos, ushort day )
{
  // Allow 53 for yearly!
  if ( mRecurReadOnly || pos > 53 || pos < -53 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;
  QValueList<RecurrenceRule::WDayPos> positions = rrule->byDays();

  RecurrenceRule::WDayPos p( pos, day );
  if ( !positions.contains( p ) ) {
    positions.append( p );
    rrule->setByDays( positions );
    updated();
  }
}


void Recurrence::addMonthlyDate( short day )
{
  if ( mRecurReadOnly || day > 31 || day < -31 ) return;
  RecurrenceRule *rrule = defaultRRule( true );
  if ( !rrule ) return;

  QValueList<int> monthDays = rrule->byMonthDays();
  if ( !monthDays.contains( day ) ) {
    monthDays.append( day );
    rrule->setByMonthDays( monthDays );
    updated();
  }
}

void Recurrence::setYearly( int freq )
{
  if ( setNewRecurrenceType( RecurrenceRule::rYearly, freq ) )
    updated();
}


// Daynumber within year
void Recurrence::addYearlyDay( int day )
{
  RecurrenceRule *rrule = defaultRRule( false ); // It must already exist!
  if ( !rrule ) return;

  QValueList<int> days = rrule->byYearDays();
  if ( !days.contains( day ) ) {
    days << day;
    rrule->setByYearDays( days );
    updated();
  }
}

// day part of date within year
void Recurrence::addYearlyDate( int day )
{
  addMonthlyDate( day );
}

// day part of date within year, given as position (n-th weekday)
void Recurrence::addYearlyPos( short pos, const QBitArray &days )
{
  addMonthlyPos( pos, days );
}


// month part of date within year
void Recurrence::addYearlyMonth( short month )
{
  if ( mRecurReadOnly || month < 1 || month > 12 ) return;
  RecurrenceRule *rrule = defaultRRule( false );
  if ( !rrule ) return;

  QValueList<int> months = rrule->byMonths();
  if ( !months.contains(month) ) {
    months << month;
    rrule->setByMonths( months );
    updated();
  }
}


TimeList Recurrence::recurTimesOn( const QDate &date ) const
{
  TimeList times;
  // The whole day is excepted
  if ( mExDates.contains( date ) ) return times;
  // EXRULE takes precedence over RDATE entries, so for floating events,
  // a matching excule also excludes the whole day automatically
  if ( doesFloat() ) {
    for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
      if ( (*rr)->recursOn( date ) )
        return times;
    }
  }

  if ( startDate() == date ) times << startDateTime().time();
  bool foundDate = false;
  for ( DateTimeList::ConstIterator it = mRDateTimes.begin();
        it != mRDateTimes.end(); ++it ) {
    if ( (*it).date() == date ) {
      times << (*it).time();
      foundDate = true;
    } else if (foundDate) break; // <= Assume that the rdatetime list is sorted
  }
  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    times += (*rr)->recurTimesOn( date );
  }
  qSortUnique( times );

  foundDate = false;
  TimeList extimes;
  for ( DateTimeList::ConstIterator it = mExDateTimes.begin();
        it != mExDateTimes.end(); ++it ) {
    if ( (*it).date() == date ) {
      extimes << (*it).time();
      foundDate = true;
    } else if (foundDate) break;
  }
  if ( !doesFloat() ) {     // we have already checked floating times above
    for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
      extimes += (*rr)->recurTimesOn( date );
    }
  }
  qSortUnique( extimes );

  for ( TimeList::Iterator it = extimes.begin(); it != extimes.end(); ++it ) {
    times.remove( (*it) );
  }
  return times;
}


QDateTime Recurrence::getNextDateTime( const QDateTime &preDateTime ) const
{
//kdDebug(5800) << " Recurrence::getNextDateTime after " << preDateTime << endl;
  QDateTime nextDT = preDateTime;
  // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
  // the exrule is identical to the rrule). If an occurrence is found, break
  // out of the loop by returning that QDateTime
// TODO_Recurrence: Is a loop counter of 1000 really okay? I mean for secondly
// recurrence, an exdate might exclude more than 1000 intervals!
  int loop = 0;
  while ( loop < 1000 ) {
    // Outline of the algo:
    //   1) Find the next date/time after preDateTime when the event could recur
    //     1.0) Add the start date if it's after preDateTime
    //     1.1) Use the next occurrence from the explicit RDATE lists
    //     1.2) Add the next recurrence for each of the RRULEs
    //   2) Take the earliest recurrence of these = QDateTime nextDT
    //   3) If that date/time is not excluded, either explicitly by an EXDATE or
    //      by an EXRULE, return nextDT as the next date/time of the recurrence
    //   4) If it's excluded, start all at 1), but starting at nextDT (instead
    //      of preDateTime). Loop at most 1000 times.
    ++loop;
    // First, get the next recurrence from the RDate lists
    DateTimeList dates;
    if ( nextDT < startDateTime() ) dates << startDateTime();
    DateTimeList::ConstIterator it = mRDateTimes.begin();
    // Assume that the rdatetime list is sorted
    while ( it != mRDateTimes.end() && (*it) <= nextDT ) ++it;
    if ( it != mRDateTimes.end() ) dates << (*it);

/*kdDebug(5800) << "    nextDT: " << nextDT << ", startDT: " << startDateTime() << endl;
kdDebug(5800) << "   getNextDateTime: found " << dates.count() << " RDATES and DTSTART in loop " << loop << endl;*/
    DateList::ConstIterator dit = mRDates.begin();
    while ( dit != mRDates.end() && QDateTime( (*dit), startDateTime().time() ) <= nextDT ) ++dit;
    if ( dit != mRDates.end() ) dates << QDateTime( (*dit), startDateTime().time() );

    // Add the next occurrences from all RRULEs.
    for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
      QDateTime dt = (*rr)->getNextDate( nextDT );
      if ( dt.isValid() ) dates << dt;
    }

    // Take the first of these (all others can't be used later on)
    qSortUnique( dates );
// kdDebug(5800) << "   getNextDateTime: found " << dates.count() << " dates in loop " << loop << endl;

    if ( dates.isEmpty() ) return QDateTime();
    nextDT = dates.first();

    // Check if that date/time is excluded explicitly or by an exrule:
    if ( !mExDates.contains( nextDT.date() ) && !mExDateTimes.contains( nextDT ) ) {
// kdDebug(5800) << "   NextDT" << nextDT << " not excluded by EXDATE " << endl;
      bool allowed = true;
      for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
        allowed = allowed && !( (*rr)->recursAt( nextDT ) );
      }
// kdDebug(5800) << "   NextDT " << nextDT << ", allowed=" << allowed << endl;
      if ( allowed ) return nextDT;
    }
  }

  // Couldn't find a valid occurrences in 1000 loops, something is wrong!
  return QDateTime();
}

QDateTime Recurrence::getPreviousDateTime( const QDateTime &afterDateTime ) const
{
  QDateTime prevDT = afterDateTime;
  // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
  // the exrule is identical to the rrule). If an occurrence is found, break
  // out of the loop by returning that QDateTime
  int loop = 0;
  while ( loop < 1000 ) {
    // Outline of the algo:
    //   1) Find the next date/time after preDateTime when the event could recur
    //     1.1) Use the next occurrence from the explicit RDATE lists
    //     1.2) Add the next recurrence for each of the RRULEs
    //   2) Take the earliest recurrence of these = QDateTime nextDT
    //   3) If that date/time is not excluded, either explicitly by an EXDATE or
    //      by an EXRULE, return nextDT as the next date/time of the recurrence
    //   4) If it's excluded, start all at 1), but starting at nextDT (instead
    //      of preDateTime). Loop at most 1000 times.
    ++loop;
    // First, get the next recurrence from the RDate lists
    DateTimeList dates;
    if ( prevDT > startDateTime() ) dates << startDateTime();

    DateTimeList::ConstIterator dtit = mRDateTimes.end();
    if ( dtit != mRDateTimes.begin() ) {
      do {
        --dtit;
      } while ( dtit != mRDateTimes.begin() && (*dtit) >= prevDT );
      if ( (*dtit) < prevDT ) dates << (*dtit);
    }

    DateList::ConstIterator dit = mRDates.end();
    if ( dit != mRDates.begin() ) {
      do {
        --dit;
      } while ( dit != mRDates.begin() && QDateTime((*dit), startDateTime().time()) >= prevDT );
      if ( QDateTime((*dit), startDateTime().time()) < prevDT )
        dates << QDateTime( (*dit), startDateTime().time() );
    }

    // Add the previous occurrences from all RRULEs.
    for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
      QDateTime dt = (*rr)->getPreviousDate( prevDT );
      if ( dt.isValid() ) dates << dt;
    }
//kdDebug(5800) << "   getPreviousDateTime: found " << dates.count() << " dates in loop " << loop << endl;

    // Take the last of these (all others can't be used later on)
    qSortUnique( dates );
    if ( dates.isEmpty() ) return QDateTime();
    prevDT = dates.last();

    // Check if that date/time is excluded explicitly or by an exrule:
    if ( !mExDates.contains( prevDT.date() ) && !mExDateTimes.contains( prevDT ) ) {
      bool allowed = true;
      for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
        allowed = allowed && !( (*rr)->recursAt( prevDT ) );
      }
      if ( allowed ) return prevDT;
    }
  }

  // Couldn't find a valid occurrences in 1000 loops, something is wrong!
  return QDateTime();
}


/***************************** PROTECTED FUNCTIONS ***************************/


RecurrenceRule::List Recurrence::rRules() const
{
  return mRRules;
}

void Recurrence::addRRule( RecurrenceRule *rrule )
{
  if ( mRecurReadOnly || !rrule ) return;
  rrule->setFloats( mFloating );
  mRRules.append( rrule );
  rrule->addObserver( this );
  updated();
}

void Recurrence::removeRRule( RecurrenceRule *rrule )
{
  if (mRecurReadOnly) return;
  mRRules.remove( rrule );
  rrule->removeObserver( this );
  updated();
}

RecurrenceRule::List Recurrence::exRules() const
{
  return mExRules;
}

void Recurrence::addExRule( RecurrenceRule *exrule )
{
  if ( mRecurReadOnly || !exrule ) return;
  exrule->setFloats( mFloating );
  mExRules.append( exrule );
  exrule->addObserver( this );
  updated();
}

void Recurrence::removeExRule( RecurrenceRule *exrule )
{
  if (mRecurReadOnly) return;
  mExRules.remove( exrule );
  exrule->removeObserver( this );
  updated();
}


DateTimeList Recurrence::rDateTimes() const
{
  return mRDateTimes;
}

void Recurrence::setRDateTimes( const DateTimeList &rdates )
{
  if ( mRecurReadOnly ) return;
  mRDateTimes = rdates;
  qSortUnique( mRDateTimes );
  updated();
}

void Recurrence::addRDateTime( const QDateTime &rdate )
{
  if ( mRecurReadOnly ) return;
  mRDateTimes.append( rdate );
  qSortUnique( mRDateTimes );
  updated();
}


DateList Recurrence::rDates() const
{
  return mRDates;
}

void Recurrence::setRDates( const DateList &rdates )
{
  if ( mRecurReadOnly ) return;
  mRDates = rdates;
  qSortUnique( mRDates );
  updated();
}

void Recurrence::addRDate( const QDate &rdate )
{
  if ( mRecurReadOnly ) return;
  mRDates.append( rdate );
  qSortUnique( mRDates );
  updated();
}


DateTimeList Recurrence::exDateTimes() const
{
  return mExDateTimes;
}

void Recurrence::setExDateTimes( const DateTimeList &exdates )
{
  if ( mRecurReadOnly ) return;
  mExDateTimes = exdates;
  qSortUnique( mExDateTimes );
}

void Recurrence::addExDateTime( const QDateTime &exdate )
{
  if ( mRecurReadOnly ) return;
  mExDateTimes.append( exdate );
  qSortUnique( mExDateTimes );
  updated();
}


DateList Recurrence::exDates() const
{
  return mExDates;
}

void Recurrence::setExDates( const DateList &exdates )
{
  if ( mRecurReadOnly ) return;
  mExDates = exdates;
  qSortUnique( mExDates );
  updated();
}

void Recurrence::addExDate( const QDate &exdate )
{
  if ( mRecurReadOnly ) return;
  mExDates.append( exdate );
  qSortUnique( mExDates );
  updated();
}

void Recurrence::recurrenceChanged( RecurrenceRule * )
{
  updated();
}


// %%%%%%%%%%%%%%%%%% end:Recurrencerule %%%%%%%%%%%%%%%%%%

void Recurrence::dump() const
{
  kdDebug(5800) << "Recurrence::dump():" << endl;

  kdDebug(5800) << "  -) " << mRRules.count() << " RRULEs: " << endl;
  for ( RecurrenceRule::List::ConstIterator rr = mRRules.begin(); rr != mRRules.end(); ++rr ) {
    kdDebug(5800) << "    -) RecurrenceRule : " << endl;
    (*rr)->dump();
  }
  kdDebug(5800) << "  -) " << mExRules.count() << " EXRULEs: " << endl;
  for ( RecurrenceRule::List::ConstIterator rr = mExRules.begin(); rr != mExRules.end(); ++rr ) {
    kdDebug(5800) << "    -) ExceptionRule : " << endl;
    (*rr)->dump();
  }


  kdDebug(5800) << endl << "  -) " << mRDates.count() << " Recurrence Dates: " << endl;
  for ( DateList::ConstIterator it = mRDates.begin(); it != mRDates.end(); ++it ) {
    kdDebug(5800) << "     " << (*it) << endl;
  }
  kdDebug(5800) << endl << "  -) " << mRDateTimes.count() << " Recurrence Date/Times: " << endl;
  for ( DateTimeList::ConstIterator it = mRDateTimes.begin(); it != mRDateTimes.end(); ++it ) {
    kdDebug(5800) << "     " << (*it) << endl;
  }
  kdDebug(5800) << endl << "  -) " << mExDates.count() << " Exceptions Dates: " << endl;
  for ( DateList::ConstIterator it = mExDates.begin(); it != mExDates.end(); ++it ) {
    kdDebug(5800) << "     " << (*it) << endl;
  }
  kdDebug(5800) << endl << "  -) " << mExDateTimes.count() << " Exception Date/Times: " << endl;
  for ( DateTimeList::ConstIterator it = mExDateTimes.begin(); it != mExDateTimes.end(); ++it ) {
    kdDebug(5800) << "     " << (*it) << endl;
  }
}
