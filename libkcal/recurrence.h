/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2002 David Jarvie <software@astrojar.org.uk>
    Copyright (C) 2005 Reinhold Kainhofer <reinhold@kainhofer.com>

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
#ifndef KCAL_RECURRENCE_H
#define KCAL_RECURRENCE_H

#include <qstring.h>
#include <qbitarray.h>
#include <qptrlist.h>

#include "libkcal_export.h"
#include "recurrencerule.h"

namespace KCal {

class RecurrenceRule;

/**
  This class represents a recurrence rule for a calendar incidence.

  It manages all recurrence rules, recurrence date/times, exception rules
  and exception date times that can appear inside calendar items.
  Each recurrence rule and exception rule is represented as an object
  of type RecurrenceRule.

  For the simple case where at most one recurrence
  rule is present, this class provides shortcut methods to set the type:
    setMinutely()
    setHourly()
    setDaily()
    setWeekly()
    setMonthly()
    setYearly()
  to set/get general information about the recurrence:
    setEndDate()
    setEndDateTime()
    duration()
    durationTo()
    setDuration()
    frequency()
    setFrequency()
  and to set/get specific information about the recurrence within the interval:
    days()
    monthDays()
    monthPositions()
    yearDays()
    yearDates()
    yearMonths()
    yearPositions()
    addMonthlyPos()
    addMonthlyDate()
    addYearlyDay()
    addYearlyDate()
    addYearlyPos()
    addYearlyMonth()
  These are all available so that you don't have to work on the RecurrenceRule
  objects themselves.
  In other words, in that simple situation the interface stays almost the
  same compared to the old Recurrence class, which allowed only one
  recurrence rule.

  As soon as your recurrence consists of multiple recurrence rules or exception
  rules, you cannot use the methods mentioned above any more (since each rule
  will have a different type and different settings). If you still call
  any of them, the set*ly methods will remove all rules and add one rule with
  the specified type. The add* and the other set* methods will change only
  the first recurrence rule, but leave the others untouched.
*/
class LIBKCAL_EXPORT Recurrence : public RecurrenceRule::Observer
{
  public:
    class Observer {
      public:
        virtual ~Observer() {}
        /** This method will be called on each change of the recurrence object */
        virtual void recurrenceUpdated( Recurrence * ) = 0;
    };

    /** enumeration for describing how an event recurs, if at all. */
    enum { rNone = 0, rMinutely = 0x001, rHourly = 0x0002, rDaily = 0x0003,
           rWeekly = 0x0004, rMonthlyPos = 0x0005, rMonthlyDay = 0x0006,
           rYearlyMonth = 0x0007, rYearlyDay = 0x0008, rYearlyPos = 0x0009,
           rOther = 0x000A, rMax=0x00FF };

    Recurrence();
    Recurrence( const Recurrence& );
    virtual ~Recurrence();

    bool operator==( const Recurrence& ) const;
    bool operator!=( const Recurrence& r ) const  { return !operator==(r); }

    /** Return the start date/time of the recurrence (Time for floating incidences will be 0:00).
     @return the current start/time of the recurrence. */
    QDateTime startDateTime() const;
    /** Return the start date/time of the recurrence */
    QDate startDate() const   { return mStartDateTime.date(); }
    /** Set start of recurrence, as a date and time. Also sets the incidence to non-floating.
       @param start the new start date/time of the incidence.
    */
    void setStartDateTime( const QDateTime &start );
    /** Set start of recurrence, as a date. Also sets the incidence to floating.
       @param start The new start date of the incidence.
    */
    void setStartDate( const QDate &start );

    /** Set whether the recurrence has no time, just a date.
     * Floating means -- according to rfc2445 -- that the event has no time
     * associated.
     * N.B. This property is derived by default from whether setStartDateTime() or
     * setStartDate() is called.
     * @return whether the recurrence has a time (false) or it is just a date (true). */
    bool doesFloat() const { return mFloating; }
    /** Sets whether the dtstart is a floating time (i.e. has no time attached)
       @param floats If the recurrence is for all-day item (true) or has a time associated (false).
       */
    void setFloats( bool floats );

    /** Set if recurrence is read-only or can be changed. */
    void setRecurReadOnly(bool readOnly) { mRecurReadOnly = readOnly; }
    /** Returns true if the recurrence is read-only, or false if it can be changed. */
    bool recurReadOnly() const  { return mRecurReadOnly; }

    /** Returns whether the event recurs at all. */
    bool doesRecur() const;
    /** Returns the event's recurrence status.  See the enumeration at the top
     * of this file for possible values. */
    ushort recurrenceType() const;
    /** Returns the recurrence status for a recurrence rule.
     * See the enumeration at the top of this file for possible values. */
    static ushort recurrenceType( const RecurrenceRule *rrule );
    /** Returns true if the date specified is one on which the event will
     * recur. */
    bool recursOn( const QDate &qd ) const;
    /** Returns true if the date/time specified is one at which the event will
     * recur. Times are rounded down to the nearest minute to determine the result. */
    bool recursAt( const QDateTime & ) const;
    /** Removes all recurrence rules. Recurrence dates and exceptions are
        not removed. */
    void unsetRecurs();
    /** Removes all recurrence and exception rules and dates. */
    void clear();

    /** Returns a list of the times on the specified date at which the
     * recurrence will occur.
     * @param date the date for which to find the recurrence times.
     */
    QValueList<QTime> recurTimesOn(const QDate &date) const;

    /** Returns the date and time of the next recurrence, after the specified date/time.
     * If the recurrence has no time, the next date after the specified date is returned.
     * @param preDateTime the date/time after which to find the recurrence.
     * @return date/time of next recurrence (strictly later than the given QDateTiem), or invalid date if none.
     */
    QDateTime getNextDateTime( const QDateTime& preDateTime ) const;
    /** Returns the date and time of the last previous recurrence, before the specified date/time.
     * If a time later than 00:00:00 is specified and the recurrence has no time, 00:00:00 on
     * the specified date is returned if that date recurs.
     * @param afterDateTime the date/time before which to find the recurrence.
     * @return date/time of previous recurrence (strictly earlier than the given QDateTime), or invalid date if none.
     */
    QDateTime getPreviousDateTime( const QDateTime& afterDateTime ) const;

    /** Returns frequency of recurrence, in terms of the recurrence time period type. */
    int frequency() const;
    /** Sets the frequency of recurrence, in terms of the recurrence time period type. */
    void setFrequency(int freq);

    /**
     * Returns -1 if the event recurs infinitely, 0 if the end date is set,
     * otherwise the total number of recurrences, including the initial occurrence.
     */
    int duration() const;
    /** Sets the total number of times the event is to occur, including both the
     * first and last. */
    void setDuration(int duration);
    /** Returns the number of recurrences up to and including the date/time specified. */
    int durationTo(const QDateTime &) const;
    /** Returns the number of recurrences up to and including the date specified. */
    int durationTo( const QDate &date ) const { return durationTo( QDateTime( date, QTime( 23, 59, 59 ) ) ); }

    /** Returns the date/time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     */
    QDateTime endDateTime() const;
    /** Returns the date of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     */
    QDate endDate() const;
    /** Sets the date of the last recurrence. The end time is set to the recurrence start time.
     * @param endDate the ending date after which to stop recurring. If the
     *   incidence is not floating, the end time will be 23:59.*/
    void setEndDate( const QDate &endDate );
    /** Sets the date and time of the last recurrence.
     * @param endDateTime the ending date/time after which to stop recurring. */
    void setEndDateTime( const QDateTime &endDateTime );



    /** Sets an event to recur minutely. By default infinite recurrence is used.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        minutely recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. 2 is every other minute
     */
    void setMinutely( int freq );

    /** Sets an event to recur hourly. By default infinite recurrence is used.
        The minute of the recurrence is taken from the start date (if you
        need to change it, you will have to modify the defaultRRule's
        byMinute list manually.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        hourly recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. 2 is every other hour
     */
    void setHourly( int freq );

    /** Sets an event to recur daily. By default infinite recurrence is used.
        The minute and second of the recurrence is taken from the start date
        (if you need to change them, you will have to modify the defaultRRule's
        byMinute list manually.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        daily recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. 2 is every other day
     */
    void setDaily( int freq );

    /** Sets an event to recur weekly. By default infinite recurrence is used.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        weekly recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. every other week etc.
     * @param weekStart the first day of the week (Monday=1 .. Sunday=7, default is Monday).
     */
    void setWeekly( int freq, int weekStart = 1 );
    /** Sets an event to recur weekly. By default infinite recurrence is used.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        weekly recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. every other week etc.
     * @param days a 7 bit array indicating which days on which to recur (bit 0 = Monday).
     * @param weekStart the first day of the week (Monday=1 .. Sunday=7, default is Monday).
     */
    void setWeekly( int freq, const QBitArray &days, int weekStart = 1 );
    /** Adds days to the weekly day recurrence list.
     * @param days a 7 bit array indicating which days on which to recur (bit 0 = Monday).
     */
    void addWeeklyDays( const QBitArray &days );
    /** Returns the first day of the week.  Uses only the
     * first RRULE if present (i.e. a second RRULE as well as all EXRULES are
     * ignored!
     * @return Weekday of the first day of the week (Monday=1 .. Sunday=7)
     */
    int weekStart() const;
    /** Returns week day mask (bit 0 = Monday). */
    QBitArray days() const; // Emulate the old behavior

    /** Sets an event to recur monthly. By default infinite recurrence is used.
        The date of the monthly recurrence will be taken from the start date
        unless you explicitly add one or more recurrence dates with
        addMonthlyDate or a recurrence position in the month (e.g. first
        monday) using addMonthlyPos.
        To set an end date use the method setEndDate and to set the number
        of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        monthly recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. 3 for every third month.
     */
    void setMonthly( int freq );
    /** Adds a position (e.g. first monday) to the monthly recurrence rule.
     * @param pos the position in the month for the recurrence, with valid
     * values being 1-5 (5 weeks max in a month).
     * @param days the days for the position to recur on (bit 0 = Monday).
     * Example: pos = 2, and bits 0 and 2 are set in days:
     * the rule is to repeat every 2nd Monday and Wednesday in the month.
     */
    void addMonthlyPos( short pos, const QBitArray &days );
    void addMonthlyPos( short pos, ushort day );
    /** Adds a date (e.g. the 15th of each month) to the monthly day
     *  recurrence list.
     * @param day the date in the month to recur.
     */
    void addMonthlyDate( short day );
    /** Returns list of day positions in months. */
    QValueList<RecurrenceRule::WDayPos> monthPositions() const;
    /** Returns list of day numbers of a  month. */
    // Emulate old behavior
    QValueList<int> monthDays() const;

    /** Sets an event to recur yearly. By default, this will recur every year
     *  on the same date (e.g. every year on April 15 if the start date was
     *  April 15).
     *  The day of the year can be specified with addYearlyDay().
     *  The day of the month can be specified with addYearlyByDate
     *  If both a month and a day ar specified with addYearlyMonth and
     *  addYearlyDay, the day is understood as day number within the month.
     *
     *  A position (e.g. 3rd Sunday of year/month, or last Friday of year/month)
     *  can be specified with addYearlyPos. Again, if a month is specified,
     *  this position is understood as within that month, otherwise within
     *  the year.
     *
     *  By default infinite recurrence is used. To set an end date use the
     *  method setEndDate and to set the number of occurrences use setDuration.

        This method clears all recurrence rules and adds one rule with a
        yearly recurrence. All other recurrence components (recurrence
        date/times, exception date/times and exception rules) are not
        modified.
     * @param freq the frequency to recur, e.g. 3 for every third year.
     */
    void setYearly( int freq );
    /** Adds day number of year within a yearly recurrence.
     *  By default infinite recurrence is used. To set an end date use the
     *  method setEndDate and to set the number of occurrences use setDuration.
     * @param day the day of the year for the event. E.g. if day is 60, this
     *            means Feb 29 in leap years and March 1 in non-leap years.
     */
    void addYearlyDay( int day );
    /** Adds date within a yearly recurrence. The month(s) for the recurrence
     *  can be specified with addYearlyMonth(), otherwise the month of the
     *  start date is used.
     *
     *   By default infinite recurrence is used. To set an end date use the
     *   method setEndDate and to set the number of occurrences use setDuration.
     * @param date the day of the month for the event
     */
    void addYearlyDate( int date );
    /** Adds month in yearly recurrence. You can specify specific day numbers
     *  within the months (by calling addYearlyDate()) or specific day positions
     *  within the month (by calling addYearlyPos).
     * @param _rNum the month in which the event shall recur.
     */
    void addYearlyMonth( short _rNum );
    /** Adds position within month/year within a yearly recurrence. If months
     *  are specified (via addYearlyMonth()), the parameters are understood as
     *  position within these months, otherwise within the year.
     *
     *  By default infinite recurrence is used.
     *   To set an end date use the method setEndDate and to set the number
     *   of occurrences use setDuration.
     * @param pos the position in the month/year for the recurrence, with valid
     * values being 1 to 53 and -1 to -53 (53 weeks max in a year).
     * @param days the days for the position to recur on (bit 0 = Monday).
     * Example: pos = 2, and bits 0 and 2 are set in days
     *   If months are specified (via addYearlyMonth), e.g. March, the rule is
     *   to repeat every year on the 2nd Monday and Wednesday of March.
     *   If no months are specified, the fule is to repeat every year on the
     *   2nd Monday and Wednesday of the year.
     */
    void addYearlyPos( short pos, const QBitArray &days );

    /** Returns the day numbers within a yearly recurrence.
     * @return the days of the year for the event. E.g. if the list contains
     *         60, this means the recurrence happens on day 60 of the year, i.e.
     *         on Feb 29 in leap years and March 1 in non-leap years.
     */
    QValueList<int> yearDays() const;
    /** Returns the dates within a yearly recurrence.
     * @return the days of the month for the event. E.g. if the list contains
     *         13, this means the recurrence happens on the 13th of the month.
     *         The months for the recurrence can be obtained through
     *         yearlyMonths(). If this list is empty, the month of the start
     *         date is used.
     */
    QValueList<int> yearDates() const;
    /** Returns the months within a yearly recurrence.
     * @return the months for the event. E.g. if the list contains
     *         11, this means the recurrence happens in November.
     *         The days for the recurrence can be obtained either through
     *         yearDates() if they are given as dates within the month or
     *         through yearlyPositions() if they are given as positions within the
     *         month. If none is specified, the date of the start date is used.
     */
    QValueList<int> yearMonths() const;
    /** Returns the positions within a yearly recurrence.
     * @return the positions for the event, either within a month (if months
     *         are set through addYearlyMonth()) or within the year.
     *         E.g. if the list contains {Pos=3, Day=5}, this means the third
     *         friday. If a month is set this position is understoodas third
     *         Friday in the given months, otherwise as third Friday of the
     *         year.
     */
    /** Returns list of day positions in months, for a recursYearlyPos recurrence rule. */
    QValueList<RecurrenceRule::WDayPos> yearPositions() const;

    /** Upper date limit for recurrences */
    static const QDate MAX_DATE;

    /**
      Debug output.
    */
    void dump() const;


    // RRULE
    RecurrenceRule::List rRules() const;
    void addRRule( RecurrenceRule *rrule );
    void removeRRule( RecurrenceRule *rrule );
    // EXRULE
    RecurrenceRule::List exRules() const;
    void addExRule( RecurrenceRule *exrule );
    void removeExRule( RecurrenceRule *exrule );

    // RDATE
    DateTimeList rDateTimes() const;
    DateList rDates() const;
    void setRDateTimes( const DateTimeList &rdates);
    void setRDates( const DateList &rdates);
    void addRDateTime( const QDateTime &rdate );
    void addRDate( const QDate &rdate );

    // ExDATE
    DateTimeList exDateTimes() const;
    DateList exDates() const;
    void setExDateTimes( const DateTimeList &exdates);
    void setExDates( const DateList &exdates);
    void addExDateTime( const QDateTime &exdate );
    void addExDate( const QDate &exdate );

    RecurrenceRule *defaultRRule( bool create = false ) const;
    RecurrenceRule *defaultRRuleConst() const;
    void updated();

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

    void recurrenceChanged( RecurrenceRule * );

  protected:
    RecurrenceRule *setNewRecurrenceType( RecurrenceRule::PeriodType type, int freq );

  private:
    RecurrenceRule::List mExRules;
    RecurrenceRule::List mRRules;
    QValueList<QDateTime>mRDateTimes;
    QValueList<QDate> mRDates;
    QValueList<QDateTime> mExDateTimes;
    QValueList<QDate> mExDates;

    QDateTime mStartDateTime;            // date/time of first recurrence
    bool mFloating;                      // the recurrence has no time, just a date
    bool mRecurReadOnly;

    // Cache the type of the recurrence with the old system (e.g. MonthlyPos)
    mutable ushort mCachedType;

    QValueList<Observer*> mObservers;

    class Private;
    Private *d;
};

}

#endif
