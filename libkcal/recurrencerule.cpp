/*
    This file is part of libkcal.

    Copyright (c) 2005 Reinhold Kainhofer <reinhold@kainhofe.com>
    Parts are taken from the Recurrence class:
    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
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

#include "recurrencerule.h"
#include "incidence.h"

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <qdatetime.h>

#include <limits.h>
#include <math.h>

using namespace KCal;





/**************************************************************************
 *                               DateHelper                               *
 **************************************************************************/


class DateHelper {
  public:
    static QString dayName( short day );
    static QDate getNthWeek( int year, int weeknumber, short weekstart );
    static int getWeekNumber( const QDate &date, short weekstart, int *year = 0 );
    static int getWeekNumberNeg( const QDate &date, short weekstart, int *year = 0 );
};


QString DateHelper::dayName( short day )
{
  switch ( day ) {
    case 1: return "MO"; break;
    case 2: return "TU"; break;
    case 3: return "WE"; break;
    case 4: return "TH"; break;
    case 5: return "FR"; break;
    case 6: return "SA"; break;
    case 7: return "SU"; break;
    default: return "??";
  }
}


QDate DateHelper::getNthWeek( int year, int weeknumber, short weekstart )
{
  if ( weeknumber == 0 ) return QDate();
  // Adjust this to the first day of week #1 of the year and add 7*weekno days.
  QDate dt( year, 1, 4 ); // Week #1 is the week that contains Jan 4
  int adjust = -(7 + dt.dayOfWeek() - weekstart) % 7;
  if ( weeknumber > 0 ) {
    dt = dt.addDays( 7 * (weeknumber-1) + adjust );
  } else {
    dt = dt.addYears( 1 );
    dt = dt.addDays( 7 * weeknumber + adjust );
  }
  return dt;
}


int DateHelper::getWeekNumber( const QDate &date, short weekstart, int *year )
{
// kdDebug(5800) << "Getting week number for " << date << " with weekstart="<<weekstart<<endl;
  if ( year ) *year = date.year();
  QDate dt( date.year(), 1, 4 ); // <= definitely in week #1
  dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
  QDate dtn( date.year()+1, 1, 4 ); // <= definitely first week of next year
  dtn = dtn.addDays( -(7 + dtn.dayOfWeek() - weekstart) % 7 );

// kdDebug(5800) << "Week one of year " << date.year() << " is: " << dt << endl;
// kdDebug(5800) << "Week one of year " << date.year()+1 << " is: " << dtn << endl;

  int daysto = dt.daysTo( date );
  int dayston = dtn.daysTo( date );
// kdDebug() << "  daysto=" << daysto << ", dayston=" << dayston << endl;
  if ( daysto < 0 ) {
    if ( year ) *year = date.year()-1;
    dt = QDate( date.year()-1, 1, 4 );
    dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
    daysto = dt.daysTo( date );
  } else if ( dayston >= 0 ) {
    // in first week of next year;
    if ( year ) *year = date.year() + 1;
    dt = dtn;
    daysto = dayston;
  }
// kdDebug(5800) << "   year=" << ((year)?(*year):0) << ", weeknr = " << daysto/7+1 << ", daysto = " << daysto << endl;
  return daysto / 7 + 1;
}


int DateHelper::getWeekNumberNeg( const QDate &date, short weekstart, int *year )
{
// TODO: Cross-check this!
// TODO: implement the case where it's in week 53 of the previous year
  QDate dt( date.year() + 1, 1, 4 ); // <= definitely in week #1
  dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
  int daysto = dt.daysTo( date ) + 1; // days to begin of week #1
  if ( daysto >= 0 ) {
    // it's week #1 of next year => relative to the end of the next year
    dt = QDate( date.year() + 2, 1, 4 );
    dt = dt.addDays( -(7 + dt.dayOfWeek() - weekstart) % 7 ); // begin of week #1
    daysto = dt.daysTo( date ) + 1;
  }
  return daysto / 7 - 1;
}





/**************************************************************************
 *                       RecurrenceRule::Constraint                       *
 **************************************************************************/


const QDate KCal::RecurrenceRule::MAX_DATE(3000, 1, 1);   // Maximum date = 1-Jan-3000

RecurrenceRule::Constraint::Constraint( int wkst )
{
  weekstart = wkst;
  clear();
}

RecurrenceRule::Constraint::Constraint( const QDateTime &preDate, PeriodType type, int wkst )
{
  weekstart = wkst;
  readDateTime( preDate, type );
}

void RecurrenceRule::Constraint::clear()
{
  year = 0;
  month = 0;
  day = 0;
  hour = -1;
  minute = -1;
  second = -1;
  weekday = 0;
  weekdaynr = 0;
  weeknumber = 0;
  yearday = 0;
}

bool RecurrenceRule::Constraint::matches( const QDate &dt, RecurrenceRule::PeriodType type ) const
{
  // If the event recurs in week 53 or 1, the day might not belong to the same
  // year as the week it is in. E.g. Jan 1, 2005 is in week 53 of year 2004.
  // So we can't simply check the year in that case!
  if ( weeknumber == 0 ) {
    if ( year > 0 && year != dt.year() ) return false;
  } else {
    int y;
// kdDebug(5800) << "Matching week number, ignoring year for now!" << endl;
    // TODO: Add a year check here
// kdDebug(5800) << " week number for " << dt << " with start " << weekstart << " is: " << DateHelper::getWeekNumber( dt, weekstart ) << endl;
// kdDebug(5800) << " Week is in year " << y << endl;
// kdDebug(5800) << " We are comparing with week " << weeknumber << "of year year" << endl;
    if ( weeknumber > 0 &&
         weeknumber != DateHelper::getWeekNumber( dt, weekstart, &y ) ) return false;
    if ( weeknumber < 0 &&
         weeknumber != DateHelper::getWeekNumberNeg( dt, weekstart, &y ) ) return false;
    if ( year > 0 && year != y ) return false;
  }

  if ( month > 0 && month != dt.month() ) return false;
  if ( day > 0 && day != dt.day() ) return false;
  if ( day < 0 && dt.day() != (dt.daysInMonth() + day + 1 ) ) return false;
  if ( weekday > 0 ) {
    if ( weekday != dt.dayOfWeek() ) return false;
    if ( weekdaynr != 0 ) {
      // Yearly
      if ( weekdaynr > 0 && type == rYearly &&
           weekdaynr != (dt.dayOfYear() - 1)/7 + 1 ) return false;
      if ( weekdaynr < 0 && type == rYearly &&
           weekdaynr != -((dt.daysInYear() - dt.dayOfYear() )/7 + 1 ) )
        return false;
      // Monthly
      if ( weekdaynr > 0 && type == rMonthly &&
           weekdaynr != (dt.day() - 1)/7 + 1 ) return false;
      if ( weekdaynr < 0 && type == rMonthly &&
           weekdaynr != -((dt.daysInMonth() - dt.day() )/7 + 1 ) )
        return false;

    }
  }
// kdDebug(5800) << "Vor dayOfYear" << endl;
  if ( yearday > 0 && yearday != dt.dayOfYear() ) return false;
  if ( yearday < 0 && yearday != dt.daysInYear() - dt.dayOfYear() + 1 ) return false;
  return true;
}

bool RecurrenceRule::Constraint::matches( const QDateTime &dt, RecurrenceRule::PeriodType type ) const
{
  if ( !matches( dt.date(), type ) ) return false;
  if ( hour >= 0 && hour != dt.time().hour() ) return false;
  if ( minute >= 0 && minute != dt.time().minute() ) return false;
  if ( second >= 0 && second != dt.time().second() ) return false;
  return true;
}

bool RecurrenceRule::Constraint::isConsistent( PeriodType /*period*/) const
{
  // TODO: Check for consistency, e.g. byyearday=3 and bymonth=10
  return true;
}

QDateTime RecurrenceRule::Constraint::intervalDateTime( RecurrenceRule::PeriodType type ) const
{
  QDateTime dt;
  dt.setTime( QTime( 0, 0, 0 ) );
  dt.setDate( QDate( year, (month>0)?month:1, (day>0)?day:1 ) );
  if ( day < 0 )
    dt = dt.addDays( dt.date().daysInMonth() + day );
  switch ( type ) {
    case rSecondly:
      dt.setTime( QTime( hour, minute, second ) ); break;
    case rMinutely:
      dt.setTime( QTime( hour, minute, 1 ) ); break;
    case rHourly:
      dt.setTime( QTime( hour, 1, 1 ) ); break;
    case rDaily:
      break;
    case rWeekly:
      dt = DateHelper::getNthWeek( year, weeknumber, weekstart ); break;
    case rMonthly:
      dt.setDate( QDate( year, month, 1 ) ); break;
    case rYearly:
      dt.setDate( QDate( year, 1, 1 ) ); break;
    default:
      break;
  }
  return dt;
}


// Required:
//
//           Y  M  D | H  Mn S | WD #WD | WN | YD
// required:
//           x       | x  x  x |        |    |
// 0) Trivial: Exact date given, maybe other restrictions
//           x  x  x | x  x  x |        |    |
// 1) Easy case: no weekly restrictions -> at most a loop through possible dates
//           x  +  + | x  x  x |  -  -  |  - |  -
// 2) Year day is given -> date known
//           x       | x  x  x |        |    |  +
// 3) week number is given -> loop through all days of that week. Further
//    restrictions will be applied in the end, when we check all dates for
//    consistency with the constraints
//           x       | x  x  x |        |  + | (-)
// 4) week day is specified ->
//           x       | x  x  x |  x  ?  | (-)| (-)
// 5) All possiblecases have already been treated, so this must be an error!

QDateTimeList RecurrenceRule::Constraint::dateTimes( RecurrenceRule::PeriodType type ) const
{
  QDateTimeList result;
  bool done = false;
  QTime tm( hour, minute, second );
  if ( !isConsistent( type ) ) return result;

  if ( !done && day > 0 && month > 0 ) {
    QDateTime dt( QDate( year, month, day ), tm );
    if ( dt.isValid() ) result.append( dt );
kdDebug(5800)<< "           Trivial case, everything given: "  << dt << endl;
    done = true;
  }
  if ( !done && day < 0 && month > 0 ) {
    QDateTime dt( QDate( year, month, 1 ), tm );
    dt = dt.addDays( dt.date().daysInMonth() + day );
    if ( dt.isValid() ) result.append( dt );
kdDebug(5800)<< "           Trivial case, everything given: "  << dt << endl;
    done = true;
  }


  if ( !done && weekday == 0 && weeknumber == 0 && yearday == 0 ) {
kdDebug(5800) << "        easy case" << endl;
    // Easy case: date is given, not restrictions by week or yearday
    uint mstart = (month>0) ? month : 1;
    uint mend = (month <= 0) ? 12 : month;
// kdDebug(5800) << "          mstart = " << mstart << ", mend = " << mend << endl;
    for ( uint m = mstart; m <= mend; ++m ) {
      uint dstart, dend;
      if ( day > 0 ) {
        dstart = dend = day;
      } else if ( day < 0 ) {
        QDate date( year, month, 1 );
        dstart = dend = date.daysInMonth() + day + 1;
      } else {
        QDate date( year, month, 1 );
        dstart = 1;
        dend = date.daysInMonth();
      }
// kdDebug(5800) << "          dstart = " << dstart << ", dend = " << dend << endl;
      for ( uint d = dstart; d <= dend; ++d ) {
        QDateTime dt( QDate( year, m, d ), tm );
kdDebug(5800) << "            dt=" << dt << endl;
        if ( dt.isValid() ) result.append( dt );
      }
    }
    done = true;
  }

  // Else: At least one of the week / yearday restrictions was given...
  // If we have a yearday (and of course a year), we know the exact date
  if ( !done && yearday != 0 ) {
    // yearday < 0 means from end of year, so we'll need Jan 1 of the next year
    QDate d( year + ((yearday>0)?0:1), 1, 1 );
    d = d.addDays( yearday - ((yearday>0)?1:0) );
kdDebug(5800) << "         yearday, dt= " << d << endl;
    result.append( QDateTime( d, tm ) );
    done = true;
  }

  // Else: If we have a weeknumber, we have at most 7 possible dates, loop through them
  if ( !done && weeknumber != 0 ) {
    QDate wst( DateHelper::getNthWeek( year, weeknumber, weekstart ) );
    if ( weekday != 0 ) {
      wst = wst.addDays( (7 + weekday - weekstart ) % 7 );
      kdDebug(5800) << "         weeknumber, dt= " << wst << endl;
      result.append( QDateTime( wst, tm ) );
    } else {
      for ( int i = 0; i < 7; ++i ) {
        kdDebug(5800) << "         weeknumber, dt= " << wst << endl;
        result.append( QDateTime( wst, tm ) );
        wst = wst.addDays( 1 );
      }
    }
    done = true;
  }

  // weekday is given
  if ( !done && weekday != 0 ) {
    QDate dt( year, 1, 1 );
    int maxloop = 53;
    if ( type == rMonthly && month > 0 ) {
      dt = QDate( year, month, 1 );
      maxloop = 5;
    }
    if ( weekdaynr < 0 ) {
      // From end of period (month, year) => relative to begin of next period
      if ( type == rMonthly )
        dt = dt.addMonths( 1 );
      else
        dt = dt.addYears( 1 );
    }
    int adj = ( 7 + weekday - dt.dayOfWeek() ) % 7;
    dt = dt.addDays( adj ); // correct first weekday of the period

kdDebug(5800) << "         weekday, relative start dt= " << dt << endl;
    if ( weekdaynr > 0 ) {
      dt = dt.addDays( ( weekdaynr - 1 ) * 7 );
kdDebug(5800) << "         weekdaynr>0, dt= " << dt << endl;
      result.append( QDateTime( dt, tm ) );
    } else if ( weekdaynr < 0 ) {
      dt = dt.addDays( weekdaynr * 7 );
kdDebug(5800) << "         weekdaynr<0, dt= " << dt << endl;
      result.append( QDateTime( dt, tm ) );
    } else {
      // loop through all possible weeks, non-matching will be filtered later
kdDebug(5800) << "         weekdaynr = 0" << endl;
      for ( int i = 0; i < maxloop; ++i ) {
kdDebug(5800) << "           appending, dt= " << dt << endl;
        result.append( QDateTime( dt, tm ) );
        dt = dt.addDays( 7 );
      }
    }
  } // weekday != 0


  // Only use those times that really match all other constraints, too
  QDateTimeList valid;
  QDateTimeList::Iterator it;
  for ( it = result.begin(); it != result.end(); ++it ) {
    if ( matches( *it, type ) ) valid.append( *it );
  }
  // Don't sort it here, would be unnecessary work. The results from all
  // constraints will be merged to one big list of the interval. Sort that one!
  return valid;
}


bool RecurrenceRule::Constraint::increase( RecurrenceRule::PeriodType type, int freq )
{
  // convert the first day of the interval to QDateTime
  // Sub-daily types need to be converted to UTC to correctly handle DST shifts
  QDateTime dt( intervalDateTime( type ) );

  // Now add the intervals
  switch ( type ) {
    case rSecondly:
      dt = dt.addSecs( freq ); break;
    case rMinutely:
      dt = dt.addSecs( 60*freq ); break;
    case rHourly:
      dt = dt.addSecs( 3600 * freq ); break;
    case rDaily:
      dt = dt.addDays( freq ); break;
    case rWeekly:
      dt = dt.addDays( 7*freq ); break;
    case rMonthly:
      dt = dt.addMonths( freq ); break;
    case rYearly:
      dt = dt.addYears( freq ); break;
    default:
      break;
  }
  // Convert back from QDateTime to the Constraint class
  readDateTime( dt, type );

  return true;
}

bool RecurrenceRule::Constraint::readDateTime( const QDateTime &preDate, PeriodType type )
{
  clear();
  switch ( type ) {
    // Really fall through! Only weekly needs to be treated differentely!
    case rSecondly:
      second = preDate.time().second();
    case rMinutely:
      minute = preDate.time().minute();
    case rHourly:
      hour = preDate.time().hour();
    case rDaily:
      day = preDate.date().day();
    case rMonthly:
      month = preDate.date().month();
    case rYearly:
      year = preDate.date().year();
      break;

    case rWeekly:
      // Determine start day of the current week, calculate the week number from that
      weeknumber = DateHelper::getWeekNumber( preDate.date(), weekstart, &year );
//       year = preDate.date().year();
      break;
    default:
      break;
  }
  return true;
}


RecurrenceRule::RecurrenceRule(Incidence *parent )
: mPeriod( rNone ), mFrequency( 0 ), mIsReadOnly( false ),
  mDoesFloat( parent ? parent->doesFloat() : false),
  mWeekStart(1), mParent(parent)
{
}

RecurrenceRule::RecurrenceRule( const RecurrenceRule &r )
{
  mRRule = r.mRRule;
  mPeriod = r.mPeriod;
  mDateStart = r.mDateStart;
  mDuration = r.mDuration;
  mDateEnd = r.mDateEnd;
  mFrequency = r.mFrequency;

  mIsReadOnly = r.mIsReadOnly;
  mDoesFloat = r.mDoesFloat;

  mBySeconds = r.mBySeconds;
  mByMinutes = r.mByMinutes;
  mByHours = r.mByHours;
  mByDays = r.mByDays;
  mByMonthDays = r.mByMonthDays;
  mByYearDays = r.mByYearDays;
  mByWeekNumbers = r.mByWeekNumbers;
  mByMonths = r.mByMonths;
  mBySetPos = r.mBySetPos;
  mWeekStart = r.mWeekStart;
  mParent = r.mParent;

  // Handle private d-pointer
//   Private *d;
  setDirty();
}

RecurrenceRule::~RecurrenceRule()
{
}

bool operator==( const RecurrenceRule::WDayPos &pos1,
                 const RecurrenceRule::WDayPos &pos2 )
{
  return ( pos1.Day == pos2.Day ) && ( pos1.Pos == pos2.Pos );
}

bool RecurrenceRule::operator==( const RecurrenceRule& r ) const
{
  if ( mRRule != r.mRRule ) return false;
  if ( mPeriod != r.mPeriod ) return false;
  if ( mDateStart != r.mDateStart ) return false;
  if ( mDuration != r.mDuration ) return false;
  if ( mDateEnd != r.mDateEnd ) return false;
  if ( mFrequency != r.mFrequency ) return false;

  if ( mIsReadOnly != r.mIsReadOnly ) return false;
  if ( mDoesFloat != r.mDoesFloat ) return false;

  if ( mBySeconds != r.mBySeconds ) return false;
  if ( mByMinutes != r.mByMinutes ) return false;
  if ( mByHours != r.mByHours ) return false;
  if ( mByDays != r.mByDays ) return false;
  if ( mByMonthDays != r.mByMonthDays ) return false;
  if ( mByYearDays != r.mByYearDays ) return false;
  if ( mByWeekNumbers != r.mByWeekNumbers ) return false;
  if ( mByMonths != r.mByMonths ) return false;
  if ( mBySetPos != r.mBySetPos ) return false;
  if ( mWeekStart != r.mWeekStart ) return false;
  if ( mParent != r.mParent ) return false;

  // Handle private d-pointer
  return true;
}

void RecurrenceRule::setRecurrenceType( PeriodType period )
{
  if ( isReadOnly() ) return;
  mPeriod = period;
  setDirty();
}


/*void RecurrenceRule::setCompatVersion( int version )
{
  mCompatVersion = version ? version : INT_MAX;
  setDirty();
}
*/
QDateTime RecurrenceRule::endDate( bool *result ) const
{
  if ( result ) *result = false;
  if ( mPeriod == rNone ) return QDateTime();
  if ( mDuration < 0 ) {
    if ( result ) result = false;
    return QDateTime();
  } else if ( mDuration == 0 ) {
    return mDateEnd;
  } else {
    // N occurences. Check if we have a full cache. If so, return the cached end date.
    if ( ! mCached ) {
      // If not enough occurences can be found (i.e. inconsistent constraints)
      if ( !buildCache() ) {
        if ( result ) result = false;
        return QDateTime();
      }
    }
    return mCachedDateEnd;
  }
  return QDateTime();
}

void RecurrenceRule::setEndDate( const QDateTime &dateTime )
{
  if ( isReadOnly() ) return;
  mDateEnd = dateTime;
  mDuration = 0; // set to 0 because there is an end date/time
  setDirty();
}

void RecurrenceRule::setDuration(int duration)
{
  if ( isReadOnly() ) return;
  mDuration = duration;
  setDirty();
}

void RecurrenceRule::clear()
{
  if ( isReadOnly() ) return;
  mPeriod = rNone;
  mBySeconds.clear();
  mByMinutes.clear();
  mByHours.clear();
  mByDays.clear();
  mByMonthDays.clear();
  mByYearDays.clear();
  mByWeekNumbers.clear();
  mByMonths.clear();
  mBySetPos.clear();
  mWeekStart = 1;

  setDirty();
}

void RecurrenceRule::setDirty()
{
  mConstraints.clear();
  buildConstraints();
  mDirty = true;
  mCached = false;
  mCachedDates.clear();
}

void RecurrenceRule::setStartDate( const QDateTime &start )
{
  if ( isReadOnly() ) return;
  mDateStart = start;
  setDirty();
}

void RecurrenceRule::setFrequency(int freq)
{
  if ( isReadOnly() || freq <= 0 ) return;
  mFrequency = freq;
  setDirty();
}

void RecurrenceRule::setBySeconds( const QValueList<int> bySeconds )
{
  if ( isReadOnly() ) return;
  mBySeconds = bySeconds;
  setDirty();
}

void RecurrenceRule::setByMinutes( const QValueList<int> byMinutes )
{
  if ( isReadOnly() ) return;
  mByMinutes = byMinutes;
  setDirty();
}

void RecurrenceRule::setByHours( const QValueList<int> byHours )
{
  if ( isReadOnly() ) return;
  mByHours = byHours;
  setDirty();
}


void RecurrenceRule::setByDays( const QValueList<WDayPos> byDays )
{
  if ( isReadOnly() ) return;
  mByDays = byDays;
  setDirty();
}

void RecurrenceRule::setByMonthDays( const QValueList<int> byMonthDays )
{
  if ( isReadOnly() ) return;
  mByMonthDays = byMonthDays;
  setDirty();
}

void RecurrenceRule::setByYearDays( const QValueList<int> byYearDays )
{
  if ( isReadOnly() ) return;
  mByYearDays = byYearDays;
  setDirty();
}

void RecurrenceRule::setByWeekNumbers( const QValueList<int> byWeekNumbers )
{
  if ( isReadOnly() ) return;
  mByWeekNumbers = byWeekNumbers;
  setDirty();
}

void RecurrenceRule::setByMonths( const QValueList<int> byMonths )
{
  if ( isReadOnly() ) return;
  mByMonths = byMonths;
  setDirty();
}

void RecurrenceRule::setBySetPos( const QValueList<int> bySetPos )
{
  if ( isReadOnly() ) return;
  mBySetPos = bySetPos;
  setDirty();
}

void RecurrenceRule::setWeekStart( short weekStart )
{
  if ( isReadOnly() ) return;
  mWeekStart = weekStart;
  setDirty();
}




// int RecurrenceRule::maxIterations() const
// {
//   /* Find the maximum number of iterations which may be needed to reach the
//    * next actual occurrence of a monthly or yearly recurrence.
//    * More than one iteration may be needed if, for example, it's the 29th February,
//    * the 31st day of the month or the 5th Monday, and the month being checked is
//    * February or a 30-day month.
//    * The following recurrences may never occur:
//    * - For rMonthlyDay: if the frequency is a whole number of years.
//    * - For rMonthlyPos: if the frequency is an even whole number of years.
//    * - For rYearlyDay, rYearlyMonth: if the frequeny is a multiple of 4 years.
//    * - For rYearlyPos: if the frequency is an even number of years.
//    * The maximum number of iterations needed, assuming that it does actually occur,
//    * was found empirically.
//    */
//   switch (recurs) {
//     case rMonthlyDay:
//       return (rFreq % 12) ? 6 : 8;
//
//     case rMonthlyPos:
//       if (rFreq % 12 == 0) {
//         // Some of these frequencies may never occur
//         return (rFreq % 84 == 0) ? 364         // frequency = multiple of 7 years
//              : (rFreq % 48 == 0) ? 7           // frequency = multiple of 4 years
//              : (rFreq % 24 == 0) ? 14 : 28;    // frequency = multiple of 2 or 1 year
//       }
//       // All other frequencies will occur sometime
//       if (rFreq > 120)
//         return 364;    // frequencies of > 10 years will hit the date limit first
//       switch (rFreq) {
//         case 23:   return 50;
//         case 46:   return 38;
//         case 56:   return 138;
//         case 66:   return 36;
//         case 89:   return 54;
//         case 112:  return 253;
//         default:   return 25;       // most frequencies will need < 25 iterations
//       }
//
//     case rYearlyMonth:
//     case rYearlyDay:
//       return 8;          // only 29th Feb or day 366 will need more than one iteration
//
//     case rYearlyPos:
//       if (rFreq % 7 == 0)
//         return 364;    // frequencies of a multiple of 7 years will hit the date limit first
//       if (rFreq % 2 == 0) {
//         // Some of these frequencies may never occur
//         return (rFreq % 4 == 0) ? 7 : 14;    // frequency = even number of years
//       }
//       return 28;
//   }
//   return 1;
// }

void RecurrenceRule::buildConstraints()
{
  mConstraints.clear();
  Constraint con;
  if ( mWeekStart > 0 ) con.weekstart = mWeekStart;
  mConstraints.append( con );

  Constraint::List tmp;
  Constraint::List::const_iterator it;
  QValueList<int>::const_iterator intit;

  #define intConstraint( list, element ) \
  if ( !list.isEmpty() ) { \
    for ( it = mConstraints.constBegin(); it != mConstraints.constEnd(); ++it ) { \
      for ( intit = list.constBegin(); intit != list.constEnd(); ++intit ) { \
        con = (*it); \
        con.element = (*intit); \
        tmp.append( con ); \
      } \
    } \
    mConstraints = tmp; \
    tmp.clear(); \
  }

  intConstraint( mBySeconds, second );
  intConstraint( mByMinutes, minute );
  intConstraint( mByHours, hour );
  intConstraint( mByMonthDays, day );
  intConstraint( mByMonths, month );
  intConstraint( mByYearDays, yearday );
  intConstraint( mByWeekNumbers, weeknumber );
  #undef intConstraint

  if ( !mByDays.isEmpty() ) {
    for ( it = mConstraints.constBegin(); it != mConstraints.constEnd(); ++it ) {
      QValueList<WDayPos>::const_iterator dayit;
      for ( dayit = mByDays.constBegin(); dayit != mByDays.constEnd(); ++dayit ) {
        con = (*it);
        con.weekday = (*dayit).Day;
        con.weekdaynr = (*dayit).Pos;
        tmp.append( con );
      }
    }
    mConstraints = tmp;
    tmp.clear();
  }

  #define fixConstraint( element, value ) \
  { \
    tmp.clear(); \
    for ( it = mConstraints.constBegin(); it != mConstraints.constEnd(); ++it ) { \
      con = (*it); con.element = value; tmp.append( con ); \
    } \
    mConstraints = tmp; \
  }
  // Now determine missing values from DTSTART. This can speed up things,
  // because we have more restrictions and save some loops.

  // TODO: Does RFC 2445 intend to restrict the weekday in all cases of weekly?
  if ( mPeriod == rWeekly && mByDays.isEmpty() ) {
    fixConstraint( weekday, mDateStart.date().dayOfWeek() );
  }

  // Really fall through in the cases, because all smaller time intervals are
  // constrained from dtstart
  switch ( mPeriod ) {
    case rYearly:
      if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() && mByYearDays.isEmpty() && mByMonths.isEmpty() ) {
        fixConstraint( month, mDateStart.date().month() );
      }
    case rMonthly:
      if ( mByDays.isEmpty() && mByWeekNumbers.isEmpty() && mByYearDays.isEmpty() && mByMonthDays.isEmpty() ) {
        fixConstraint( day, mDateStart.date().day() );
      }

    case rWeekly:
    case rDaily:
      if ( mByHours.isEmpty() ) {
        fixConstraint( hour, mDateStart.time().hour() );
      }
    case rHourly:
      if ( mByMinutes.isEmpty() ) {
        fixConstraint( minute, mDateStart.time().minute() );
      }
    case rMinutely:
      if ( mBySeconds.isEmpty() ) {
        fixConstraint( second, mDateStart.time().second() );
      }
    case rSecondly:
    default:
      break;
  }
  #undef fixConstraint

  Constraint::List::Iterator conit = mConstraints.begin();
  while ( conit != mConstraints.end() ) {
    if ( (*conit).isConsistent( mPeriod ) ) {
      ++conit;
    } else {
      conit = mConstraints.remove( conit );
    }
  }
}

bool RecurrenceRule::buildCache() const
{
kdDebug(5800) << "    Building Cache!" << endl;
  // Build the list of all occurences of this event (we need that to determine
  // the end date!)
  Constraint interval( getNextValidDateInterval( startDate(), recurrenceType() ) );
  QDateTime next;

  QDateTimeList dts = datesForInterval( interval, recurrenceType() );
  QDateTimeList::Iterator it = dts.begin();
  // Only use dates after the event has started
  while ( it != dts.end() ) {
    if ( (*it) <= startDate() ) it =  dts.remove( it );
    else ++it;
  }
  dts.prepend( startDate() ); // the start date is always the first occurence

kdDebug(5800) << "after first interval dts has # entries: " << dts.count() << endl;

  int loopnr = 0;
  int dtnr = dts.count();
  // some validity checks to avoid infinite loops (i.e. if we have
  // done this loop already 10000 times and found no occurrence, bail out )
  while ( loopnr < 10000 && dtnr < mDuration ) {
    interval.increase( recurrenceType(), frequency() );
    // The returned date list is already sorted!
    dts += datesForInterval( interval, recurrenceType() );
    dtnr = dts.count();
    ++loopnr;
  }
kdDebug(5800) << "after all intervals dts has # entries: " << dts.count() << endl;
  if ( int(dts.count()) > mDuration ) {
    // we have picked up more occurences than necessary, remove them
    it = dts.at( mDuration );
    while ( it != dts.end() ) it = dts.remove( it );
  }
  mCached = true;
  mCachedDates = dts;

kdDebug(5800) << "    Finished Building Cache, cache has " << dts.count() << " entries:" << endl;
it = dts.begin();
while ( it != dts.end() ) {
  kdDebug(5800) << "            -=> " << (*it) << endl;
  ++it;
}
  if ( int(dts.count()) == mDuration ) {
    mCachedDateEnd = dts.last();
    return true;
  } else {
    mCachedDateEnd = QDateTime();
    return false;
  }
}

QDateTime RecurrenceRule::getNextDate( const QDateTime &preDate, bool *last ) const
{
  if ( mDuration >= 0 && endDate().isValid() && preDate >= endDate() ) {
    // Beyond end of recurrence
    if (last) *last = true;
    return QDateTime();
  }

  if ( preDate < startDate() ) return startDate();
  if ( mDuration > 0 ) {
    if ( !mCached ) buildCache();
    QDateTimeList::ConstIterator it = mCachedDates.begin();
    while ( it != mCachedDates.end() && (*it) < preDate ) ++it;
    if ( it != mCachedDates.end() ) return (*it);
  }

kdDebug(5800) << "    getNext date after " << preDate << endl;
  Constraint interval( getNextValidDateInterval( preDate, recurrenceType() ) );
  bool found = false;
  bool invalid = false;
  QDateTime next;
  QDateTimeList dts = datesForInterval( interval, recurrenceType() );
  QDateTimeList::Iterator dtit = dts.begin();
  while ( dtit != dts.end() && (*dtit) <= preDate ) ++dtit;
  if ( dtit != dts.end() ) {
    next = (*dtit);
    found = true;
  }
kdDebug(5800) << "  --- next: " << next << endl;

  int loopnr = 0;
  while ( !found && !invalid ) {
    // TODO: some validity checks to avoid infinite loops (e.g. if we have
    //       done this loop already 10000 times and found no occurrence,
    //       simply bail out). This can happen if the constraints
    invalid = (loopnr>10000);
    if ( !invalid ) {
      interval.increase( recurrenceType(), frequency() );
      // The returned date list is sorted
      QDateTimeList dts = datesForInterval( interval, recurrenceType() );
      dtit = dts.begin();
      while ( dtit != dts.end() && (*dtit) <= preDate ) ++dtit;
      if ( dtit != dts.end() ) {
        next = (*dtit);
        found = true;
      }

      ++loopnr;
    }
  }
  if ( found && mDuration >= 0 && next > endDate() ) {
    // The recurrence has an end, and the next occurence would be after that
    found = false;
  }
  if ( last ) *last = !found;
  if ( found )
    return next;
  else
    return QDateTime();
}

RecurrenceRule::Constraint RecurrenceRule::getNextValidDateInterval( const QDateTime &preDate, PeriodType type ) const
{
kdDebug(5800) << "       (o) getNextValidDateInterval after " << preDate << ", type=" << type << endl;
  long periods = 0;
  QDateTime nextValid = startDate();
  int modifier = 1;
  QDateTime toDate( preDate );
  // for super-daily recurrences, don't care about the time part

  // Find the #intervals since the dtstart and round to the next multiple of
  // the frequency
      // FIXME: All sub-daily periods need to convert to UTC, do the calculations
      //        in UTC, then convert back to the local time zone. Otherwise,
      //        recurrences across DST changes will be determined wrongly
  switch ( type ) {
    // Really fall through for sub-daily, since the calculations only differ
    // by the factor 60 and 60*60! Same for weekly and daily (factor 7)
    case rHourly:   modifier *= 60;
    case rMinutely: modifier *= 60;
    case rSecondly:
        periods = startDate().secsTo( toDate ) / modifier;
        if ( periods > 0 )
          periods += ( frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid = nextValid.addSecs( modifier * periods );
        break;

    case rWeekly:
        toDate = toDate.addDays( -(7 + toDate.date().dayOfWeek() - mWeekStart) % 7 );
        modifier *= 7;
    case rDaily:
        periods = startDate().daysTo( toDate ) / modifier;
        if ( periods > 0 )
          periods += (frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid = nextValid.addDays( modifier * periods );
        break;

    case rMonthly: {
        periods = 12*( toDate.date().year() - startDate().date().year() ) +
             ( toDate.date().month() - startDate().date().month() );
        if ( periods > 0 )
          periods += (frequency() - 1 - ( (periods - 1) % frequency() ) );
        // set the day to the first day of the month, so we don't have problems
        // with non-existent days like Feb 30 or April 31
        nextValid.setDate( QDate( nextValid.date().year(), nextValid.date().month(), 1 ) );
        nextValid.setDate( nextValid.date().addMonths( periods ) );
        break; }
    case rYearly:
        periods = ( toDate.date().year() - startDate().date().year() );
        if ( periods > 0 )
          periods += ( frequency() - 1 - ( (periods - 1) % frequency() ) );
        nextValid.setDate( nextValid.date().addYears( periods ) );
        break;
    default:
        break;
  }
kdDebug(5800) << "    ~~~> date in next interval is: : " << nextValid << endl;

  return Constraint( nextValid, type, mWeekStart );
}

bool RecurrenceRule::mergeIntervalConstraint( Constraint *merged,
          const Constraint &conit, const Constraint &interval ) const
{
  Constraint result( interval );

#define mergeConstraint( name, cmparison ) \
  if ( conit.name cmparison ) { \
    if ( !(result.name cmparison) || result.name == conit.name ) { \
      result.name = conit.name; \
    } else return false;\
  }

  mergeConstraint( year, > 0 );
  mergeConstraint( month, > 0 );
  mergeConstraint( day, != 0 );
  mergeConstraint( hour, >= 0 );
  mergeConstraint( minute, >= 0 );
  mergeConstraint( second, >= 0 );

  mergeConstraint( weekday, != 0 );
  mergeConstraint( weekdaynr, != 0 );
  mergeConstraint( weeknumber, != 0 );
  mergeConstraint( yearday, != 0 );

  #undef mergeConstraint
  if ( merged ) *merged = result;
  return true;
}


QDateTimeList RecurrenceRule::datesForInterval( const Constraint &interval, PeriodType type ) const
{
  /* -) Loop through constraints,
     -) merge interval with each constraint
     -) if merged constraint is not consistent => ignore that constraint
     -) if complete => add that one date to the date list
     -) Loop through all missing fields => For each add the resulting
  */
kdDebug(5800) << "         Interval: " << endl;
interval.dump();
  QDateTimeList lst;
  Constraint::List::ConstIterator conit = mConstraints.begin();
  for ( ; conit != mConstraints.end(); ++conit ) {
    Constraint merged;
    bool mergeok = mergeIntervalConstraint( &merged, *conit, interval );
    // If the information is incomplete, we can't use this constraint
    if ( merged.year <= 0 || merged.hour < 0 || merged.minute < 0 || merged.second < 0 )
      mergeok = false;
    if ( mergeok ) {
      kdDebug(5800) << "      -) merged constraint: " << endl;
      merged.dump();
      // We have a valid constraint, so get all datetimes that match it andd
      // append it to all date/times of this interval
// kdDebug(5800)<<" We have a valid constraint!" << endl;
      QDateTimeList lstnew = merged.dateTimes( type );
      lst += lstnew;
    }
  }
  // Sort it so we can apply the BySetPos. Also some logic relies on this being sorted
  qHeapSort( lst );


if ( lst.isEmpty() ) {
  kdDebug(5800) << "         No Dates in Interval " << endl;
} else {
  kdDebug(5800) << "         Dates: " << endl;
  for ( QDateTimeList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
    kdDebug(5800)<< "              -) " << (*it).toString() << endl;
  }
  kdDebug(5800) << "       ---------------------" << endl;
}
  // TODO: make sure lst contains only unique datetimes!
  if ( !mBySetPos.isEmpty() ) {
    QDateTimeList tmplst = lst;
    lst.clear();
    QValueList<int>::ConstIterator it;
    for ( it = mBySetPos.begin(); it != mBySetPos.end(); ++it ) {
      int pos = *it;
      if ( pos > 0 ) --pos;
      if ( pos < 0 ) pos += tmplst.count();
      if ( pos >= 0 && uint(pos) < tmplst.count() ) {
        lst.append( tmplst[pos] );
      }
    }
    qHeapSort( lst );
    // TODO: make sure each DateTime appears just once!
  }

  return lst;
}


void RecurrenceRule::dump() const
{
  kdDebug(5800) << "RecurrenceRule::dump():" << endl;
  if ( !mRRule.isEmpty() )
    kdDebug(5800) << "   RRULE=" << mRRule << endl;
  kdDebug(5800) << "   Read-Only: " << isReadOnly() <<
                   ", dirty: " << mDirty << endl;

  kdDebug(5800) << "   Period type: " << recurrenceType() << ", frequency: " << frequency() << endl;
  kdDebug(5800) << "   #occurrences: " << duration() << endl;
  kdDebug(5800) << "   start date: " << startDate() <<", end date: " << endDate() << endl;


#define dumpByIntList(list,label) \
  if ( !list.isEmpty() ) {\
    QStringList lst;\
    for ( QValueList<int>::ConstIterator it = list.begin();\
          it != list.end(); ++it ) {\
      lst.append( QString::number( *it ) );\
    }\
    kdDebug(5800) << "   " << label << lst.join(", ") << endl;\
  }
  dumpByIntList( mBySeconds,    "BySeconds:  " );
  dumpByIntList( mByMinutes,    "ByMinutes:  " );
  dumpByIntList( mByHours,      "ByHours:    " );
  if ( !mByDays.isEmpty() ) {
    QStringList lst;
    for ( QValueList<WDayPos>::ConstIterator it = mByDays.begin();
          it != mByDays.end(); ++it ) {
      lst.append( ( ((*it).Pos!=0) ? QString::number( (*it).Pos ) : "" ) +
                   DateHelper::dayName( (*it).Day ) );
    }
    kdDebug(5800) << "   ByDays:     " << lst.join(", ") << endl;
  }
  dumpByIntList( mByMonthDays,  "ByMonthDays:" );
  dumpByIntList( mByYearDays,   "ByYearDays: " );
  dumpByIntList( mByWeekNumbers,"ByWeekNr:   " );
  dumpByIntList( mByMonths,     "ByMonths:   " );
  dumpByIntList( mBySetPos,     "BySetPos:   " );
  #undef dumpByIntList

  kdDebug(5800) << "   Week start: " << DateHelper::dayName( mWeekStart ) << endl;

  kdDebug(5800) << "   Constraints:" << endl;
  // dump constraints
  for ( Constraint::List::ConstIterator it = mConstraints.begin();
        it!=mConstraints.end(); ++it ) {
    (*it).dump();
  }
}

void RecurrenceRule::Constraint::dump() const
{
  kdDebug(5800) << "     ~> Y="<<year<<", M="<<month<<", D="<<day<<", H="<<hour<<", m="<<minute<<", S="<<second<<", wd="<<weekday<<",#wd="<<weekdaynr<<", #w="<<weeknumber<<", yd="<<yearday<<endl;
}
