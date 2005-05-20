/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown <pbrown@kde.org>
    Copyright (c) 2001,2003 Cornelius Schumacher <schumacher@kde.org>
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
#ifndef KCAL_RECURRENCERULE_H
#define KCAL_RECURRENCERULE_H

#include <qdatetime.h>
#include <qvaluelist.h>

#include "libkcal_export.h"

namespace KCal {

#define QDateTimeList QValueList<QDateTime>
#define QDateList QValueList<QDate>
#define QTimeList QValueList<QTime>


class Incidence;

/**
  This class represents a recurrence rule for a calendar incidence.
*/
class LIBKCAL_EXPORT RecurrenceRule
{
  public:
    /** enum for describing the frequency how an event recurs, if at all. */
    enum PeriodType { rNone = 0,
           rSecondly, rMinutely, rHourly,
           rDaily, rWeekly, rMonthly, rYearly
         };
    /** structure for describing the n-th weekday of the month/year. */
    struct WDayPos {
      short Day;  // Weekday, 1=monday, 7=sunday
      int Pos;    // week of the day (-1 for last, 1 for first, 0 for all weeks)
                   // Bounded by -366 and +366, 0 means all weeks in that period
    };

    RecurrenceRule( Incidence *parent/*, int compatVersion = 0*/ );
    RecurrenceRule(const RecurrenceRule&);
    ~RecurrenceRule();

    bool operator==( const RecurrenceRule& ) const;
    bool operator!=( const RecurrenceRule& r ) const  { return !operator==(r); }
    RecurrenceRule &operator=(const RecurrenceRule&);

    Incidence *parent() const { return mParent; }
    /** Set the calendar file version for backwards compatibility.
     * @param version is the KOrganizer/libkcal version, e.g. 220 for KDE 2.2.0.
     * Specify version = 0 to cancel compatibility mode.
     */
//     void setCompatVersion(int version = 0);



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
    QDateTime startDate() const   { return mDateStart; }
    /** Set start of recurrence, as a date and time. */
    void setStartDate(const QDateTime &start);


    /** Returns the date and time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     * @param result if non-null, *result is updated to true if successful,
     * or false if there is no recurrence.
     */
    QDateTime endDate( bool* result = 0 ) const;
    /** Sets the date and time of the last recurrence.
     * @param endDateTime the ending date/time after which to stop recurring. */
    void setEndDate(const QDateTime &endDateTime);


    /**
     * Returns -1 if the event recurs infinitely, 0 if the end date is set,
     * otherwise the total number of recurrences, including the initial occurrence.
     */
    int duration() const { return mDuration; }
    /** Sets the total number of times the event is to occur, including both the
     * first and last. */
    void setDuration(int duration);
    /** Returns the number of recurrences up to and including the date specified. */
    int durationTo(const QDate &) const;
    /** Returns the number of recurrences up to and including the date/time specified. */
    int durationTo(const QDateTime &) const;



    /** Returns true if the date specified is one on which the event will
     * recur. */
    bool recursOnPure(const QDate &qd) const;
    /** Returns true if the date/time specified is one at which the event will
     * recur. Times are rounded down to the nearest minute to determine the result. */
    bool recursAtPure(const QDateTime &) const;


    /** Returns a list of the times on the specified date at which the
     * recurrence will occur.
     * @param date the date for which to find the recurrence times.
     */
    QValueList<QTime> recurTimesOn(const QDate &date) const;


    /** Returns the date of the next recurrence, after the specified date.
     * @param preDate the date after which to find the recurrence.
     * @param last if non-null, *last is set to true if the next recurrence is the
     * last recurrence, else false.
     * Reply = date of next recurrence, or invalid date if none.
     */
//     QDate getNextDate(const QDate& preDate, bool* last = 0) const;
    /** Returns the date and time of the next recurrence, after the specified date/time.
     * If the recurrence has no time, the next date after the specified date is returned.
     * @param preDateTime the date/time after which to find the recurrence.
     * @param last if non-null, *last is set to true if the next recurrence is the
     * last recurrence, else false.
     * Reply = date/time of next recurrence, or invalid date if none.
     */
    QDateTime getNextDate(const QDateTime& preDateTime, bool* last = 0) const;
    /** Returns the date of the last previous recurrence, before the specified date.
     * @param afterDate the date before which to find the recurrence.
     * @param last if non-null, *last is set to true if the previous recurrence is the
     * last recurrence, else false.
     * Reply = date of previous recurrence, or invalid date if none.
     */
    QDate getPreviousDate(const QDate& afterDate, bool* last = 0) const;
    /** Returns the date and time of the last previous recurrence, before the specified date/time.
     * If a time later than 00:00:00 is specified and the recurrence has no time, 00:00:00 on
     * the specified date is returned if that date recurs.
     * @param afterDateTime the date/time before which to find the recurrence.
     * @param last if non-null, *last is set to true if the previous recurrence is the
     * last recurrence, else false.
     * Reply = date/time of previous recurrence, or invalid date if none.
     */
    QDateTime getPreviousDateTime(const QDateTime& afterDateTime, bool* last = 0) const;




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


    /** Upper date limit for recurrences */
    static const QDate MAX_DATE;

    void setDirty();

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
        QDateTimeList dateTimes( PeriodType type ) const;
        void dump() const;
    };

    Constraint getNextValidDateInterval( const QDateTime &preDate, PeriodType type ) const;
    QDateTimeList datesForInterval( const Constraint &interval, PeriodType type ) const;
    bool mergeIntervalConstraint( Constraint *merged, const Constraint &conit,
                                  const Constraint &interval ) const;
    bool buildCache() const;


    PeriodType mPeriod;
    QDateTime mDateStart;
    /** how often it recurs (including dtstart):
          -1 means infinitely,
           0 means an explicit end date,
           positive values give the number of occurences */
    int mDuration;
    QDateTime mDateEnd;
    uint mFrequency;

    bool mIsReadOnly;
    bool mDoesFloat;

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

    // Cache for duration
    mutable QDateTimeList mCachedDates;
    mutable bool mCached;
    mutable QDateTime mCachedDateEnd;

    // Backwards compatibility for KDE < 3.1.
//     int   mCompatVersion;  // calendar file version for backwards compatibility
//     short mCompatRecurs;   // original 'recurs' in old calendar format, or rNone
//     int   mCompatDuration; // original 'rDuration' in old calendar format, or 0

    Incidence *mParent;

    class Private;
    Private *d;
};

//bool operator==( const RecurrenceRule::WDayPos &pos1, const RecurrenceRule::WDayPos &pos2 );

}

#endif
