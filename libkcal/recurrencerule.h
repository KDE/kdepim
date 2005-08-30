/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
    Copyright (c) 2005, Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_RECURRENCERULE_H
#define KCAL_RECURRENCERULE_H

#include <qdatetime.h>
#include <libkcal/listbase.h>

#include "libkcal_export.h"

template <class T>
Q_INLINE_TEMPLATES void qSortUnique( QValueList<T> &lst )
{
  qHeapSort( lst );
  if ( lst.isEmpty() ) return;
  // Remove all duplicates from the times list
  // TODO: Make this more efficient!
  QValueListIterator<T> it = lst.begin();
  T last = *it;
  ++it;
  T newlast;
  while ( it != lst.end() ) {
    newlast = (*it);
    if ( newlast == last ) it = lst.remove( it );
    else {
      last = newlast;
      ++it;
    }
  }
}


namespace KCal {

typedef QValueList<QDateTime> DateTimeList;
typedef QValueList<QDate> DateList;
typedef QValueList<QTime> TimeList;




/**
  This class represents a recurrence rule for a calendar incidence.
*/
class LIBKCAL_EXPORT RecurrenceRule
{
  public:
    class Observer {
      public:
        virtual ~Observer() {}
        /** This method will be called on each change of the recurrence object */
        virtual void recurrenceChanged( RecurrenceRule * ) = 0;
    };
    typedef ListBase<RecurrenceRule> List;
    /** enum for describing the frequency how an event recurs, if at all. */
    enum PeriodType { rNone = 0,
           rSecondly, rMinutely, rHourly,
           rDaily, rWeekly, rMonthly, rYearly
         };
    /** structure for describing the n-th weekday of the month/year. */
    class WDayPos {
    public:
      WDayPos( int ps = 0 , short dy = 0 ) : mDay(dy), mPos(ps) {}
      short day() const { return mDay; }
      int pos() const { return mPos; }
      void setDay( short dy ) { mDay = dy; }
      void setPos( int ps ) { mPos = ps; }

      bool operator==( const RecurrenceRule::WDayPos &pos2 ) const {
          return ( mDay == pos2.mDay ) && ( mPos == pos2.mPos );
        }
    protected:
      short mDay;  // Weekday, 1=monday, 7=sunday
      int mPos;    // week of the day (-1 for last, 1 for first, 0 for all weeks)
                   // Bounded by -366 and +366, 0 means all weeks in that period
    };

    RecurrenceRule( /*Incidence *parent, int compatVersion = 0*/ );
    RecurrenceRule(const RecurrenceRule&);
    ~RecurrenceRule();

    bool operator==( const RecurrenceRule& ) const;
    bool operator!=( const RecurrenceRule& r ) const  { return !operator==(r); }
    RecurrenceRule &operator=(const RecurrenceRule&);

//     Incidence *parent() const { return mParent; }


    /** Set if recurrence is read-only or can be changed. */
    void setReadOnly(bool readOnly) { mIsReadOnly = readOnly; }
    /** Returns true if the recurrence is read-only, or false if it can be changed. */
    bool isReadOnly() const  { return mIsReadOnly; }


    /** Returns the event's recurrence status.  See the enumeration at the top
     * of this file for possible values. */
    bool doesRecur() const { return mPeriod!=rNone; }
    void setRecurrenceType( PeriodType period );
    PeriodType recurrenceType() const { return mPeriod; }
    /** Turns off recurrence for the event. */
    void clear();


    /** Returns frequency of recurrence, in terms of the recurrence time period type. */
    uint frequency() const { return mFrequency; }
    /** Sets the frequency of recurrence, in terms of the recurrence time period type. */
    void setFrequency( int freq );


    /** Return the start of the recurrence */
    QDateTime startDt() const   { return mDateStart; }
    /** Set start of recurrence, as a date and time. */
    void setStartDt(const QDateTime &start);

    /** Returns whether the start date has no time associated. Floating
        means -- according to rfc2445 -- that the event has no time associate. */
    bool doesFloat() const { return mFloating; }
    /** Sets whether the dtstart is a floating time (i.e. has no time attached) */
    void setFloats( bool floats );


    /** Returns the date and time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     * @param result if non-null, *result is updated to true if successful,
     * or false if there is no recurrence.
     */
    QDateTime endDt( bool* result = 0 ) const;
    /** Sets the date and time of the last recurrence.
     * @param endDateTime the ending date/time after which to stop recurring. */
    void setEndDt(const QDateTime &endDateTime);


    /**
     * Returns -1 if the event recurs infinitely, 0 if the end date is set,
     * otherwise the total number of recurrences, including the initial occurrence.
     */
    int duration() const { return mDuration; }
    /** Sets the total number of times the event is to occur, including both the
     * first and last. */
    void setDuration(int duration);
//     /** Returns the number of recurrences up to and including the date specified. */
//     int durationTo(const QDate &) const;
    /** Returns the number of recurrences up to and including the date/time specified. */
    int durationTo(const QDateTime &) const;
    /** Returns the number of recurrences up to and including the date specified. */
    int durationTo( const QDate &date ) const { return durationTo( QDateTime( date, QTime( 23, 59, 59 ) ) ); }



    /** Returns true if the date specified is one on which the event will
     * recur. The start date returns true only if it actually matches the rule. */
    bool recursOn( const QDate &qd ) const;
    /** Returns true if the date/time specified is one at which the event will
     * recur. Times are rounded down to the nearest minute to determine the result.
     * The start date/time returns true only if it actually matches the rule. */
    bool recursAt( const QDateTime & ) const;
    /** Returns true if the date matches the rules. It does not necessarily
        mean that this is an actual occurrence. In particular, the method does
        not check if the date is after the end date, or if the frequency interval
        matches */
    bool dateMatchesRules( const QDateTime &qdt ) const;


    /** Returns a list of the times on the specified date at which the
     * recurrence will occur.
     * @param date the date for which to find the recurrence times.
     */
    TimeList recurTimesOn( const QDate &date ) const;


    /** Returns the date and time of the next recurrence, after the specified date/time.
     * If the recurrence has no time, the next date after the specified date is returned.
     * @param preDateTime the date/time after which to find the recurrence.
     * @return date/time of next recurrence, or invalid date if none.
     */
    QDateTime getNextDate( const QDateTime& preDateTime ) const;
    /** Returns the date and time of the last previous recurrence, before the specified date/time.
     * If a time later than 00:00:00 is specified and the recurrence has no time, 00:00:00 on
     * the specified date is returned if that date recurs.
     * @param afterDateTime the date/time before which to find the recurrence.
     * @return date/time of previous recurrence, or invalid date if none.
     */
    QDateTime getPreviousDate( const QDateTime& afterDateTime ) const;




    void setBySeconds( const QValueList<int> bySeconds );
    void setByMinutes( const QValueList<int> byMinutes );
    void setByHours( const QValueList<int> byHours );

    void setByDays( const QValueList<WDayPos> byDays );
    void setByMonthDays( const QValueList<int> byMonthDays );
    void setByYearDays( const QValueList<int> byYearDays );
    void setByWeekNumbers( const QValueList<int> byWeekNumbers );
    void setByMonths( const QValueList<int> byMonths );
    void setBySetPos( const QValueList<int> bySetPos );
    void setWeekStart( short weekStart );

    const QValueList<int> &bySeconds() const { return mBySeconds; }
    const QValueList<int> &byMinutes() const { return mByMinutes; }
    const QValueList<int> &byHours() const { return mByHours; }

    const QValueList<WDayPos> &byDays() const { return mByDays; }
    const QValueList<int> &byMonthDays() const { return mByMonthDays; }
    const QValueList<int> &byYearDays() const { return mByYearDays; }
    const QValueList<int> &byWeekNumbers() const { return mByWeekNumbers; }
    const QValueList<int> &byMonths() const { return mByMonths; }
    const QValueList<int> &bySetPos() const { return mBySetPos; }
    short weekStart() const { return mWeekStart; }


    void setDirty();
    /**
      Installs an observer. Whenever some setting of this recurrence
      object is changed, the recurrenceUpdated( Recurrence* ) method
      of each observer will be called to inform it of changes.
      @param observer the Recurrence::Observer-derived object, which
      will be installed as an observer of this object.
    */
    void addObserver( Observer *observer );
    /**
      Removes an observer that was added with addObserver. If the
      given object was not an observer, it does nothing.
      @param observer the Recurrence::Observer-derived object to
      be removed from the list of observers of this object.
    */
    void removeObserver( Observer *observer );

    /**
      Debug output.
    */
    void dump() const;
    QString mRRule;

  private:
    class Constraint {
      public:
        typedef QValueList<Constraint> List;

        Constraint( int wkst = 1 );
/*         Constraint( const Constraint &con ) :
                     year(con.year), month(con.month), day(con.day),
                     hour(con.hour), minute(con.minute), second(con.second),
                     weekday(con.weekday), weeknumber(con.weeknumber),
                     yearday(con.yearday), weekstart(con.weekstart) {}*/
        Constraint( const QDateTime &preDate, PeriodType type, int wkst );
        void clear();

        int year;       // 0 means unspecified
        int month;      // 0 means unspecified
        int day;        // 0 means unspecified
        int hour;       // -1 means unspecified
        int minute;     // -1 means unspecified
        int second;     // -1 means unspecified
        int weekday;    //  0 means unspecified
        int weekdaynr;  // index of weekday in month/year (0=unspecified)
        int weeknumber; //  0 means unspecified
        int yearday;    //  0 means unspecified
        int weekstart;  //  first day of week (1=monday, 7=sunday, 0=unspec.)

        bool readDateTime( const QDateTime &preDate, PeriodType type );
        bool matches( const QDate &dt, RecurrenceRule::PeriodType type ) const;
        bool matches( const QDateTime &dt, RecurrenceRule::PeriodType type ) const;
        bool isConsistent() const;
        bool isConsistent( PeriodType period ) const;
        bool increase( PeriodType type, int freq );
        QDateTime intervalDateTime( PeriodType type ) const;
        DateTimeList dateTimes( PeriodType type ) const;
        void dump() const;
    };

    Constraint getNextValidDateInterval( const QDateTime &preDate, PeriodType type ) const;
    Constraint getPreviousValidDateInterval( const QDateTime &preDate, PeriodType type ) const;
    DateTimeList datesForInterval( const Constraint &interval, PeriodType type ) const;
    bool mergeIntervalConstraint( Constraint *merged, const Constraint &conit,
                                  const Constraint &interval ) const;
    bool buildCache() const;


    PeriodType mPeriod;
    QDateTime mDateStart;
    /** how often it recurs (including dtstart):
          -1 means infinitely,
           0 means an explicit end date,
           positive values give the number of occurrences */
    int mDuration;
    QDateTime mDateEnd;
    uint mFrequency;

    bool mIsReadOnly;
    bool mFloating;

    QValueList<int> mBySeconds;     // values: second 0-59
    QValueList<int> mByMinutes;     // values: minute 0-59
    QValueList<int> mByHours;       // values: hour 0-23

    QValueList<WDayPos> mByDays;   // n-th weekday of the month or year
    QValueList<int> mByMonthDays;   // values: day -31 to -1 and 1-31
    QValueList<int> mByYearDays;    // values: day -366 to -1 and 1-366
    QValueList<int> mByWeekNumbers; // values: week -53 to -1 and 1-53
    QValueList<int> mByMonths;      // values: month 1-12
    QValueList<int> mBySetPos;      // values: position -366 to -1 and 1-366
    short mWeekStart;               // first day of the week (1=Monday, 7=Sunday)

    Constraint::List mConstraints;
    void buildConstraints();
    bool mDirty;
    QValueList<Observer*> mObservers;

    // Cache for duration
    mutable DateTimeList mCachedDates;
    mutable bool mCached;
    mutable QDateTime mCachedDateEnd;

    class Private;
    Private *d;
};

}

#endif
