/*
    This file is part of libkcal.

    Copyright (c) 1998 Preston Brown
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
#ifndef KCAL_RECURRENCE_H
#define KCAL_RECURRENCE_H

#include <qstring.h>
#include <qbitarray.h>
#include <qptrlist.h>

#include <kdepimmacros.h>
namespace KCal {

class Incidence;

/**
  This class represents a recurrence rule for a calendar incidence.
*/
class KDE_EXPORT Recurrence
{
  public:
    /** enumeration for describing how an event recurs, if at all. */
    enum { rNone = 0, rMinutely = 0x001, rHourly = 0x0002, rDaily = 0x0003,
           rWeekly = 0x0004, rMonthlyPos = 0x0005, rMonthlyDay = 0x0006,
           rYearlyMonth = 0x0007, rYearlyDay = 0x0008, rYearlyPos = 0x0009 };

    /** Enumeration for specifying what date yearly recurrences of February 29th occur
     * in non-leap years. */
    enum Feb29Type {
           rMar1,    // recur on March 1st (default)
           rFeb28,   // recur on February 28th
           rFeb29    // only recur on February 29th, i.e. don't recur in non-leap years
    };

    /** structure for Recurs rMonthlyPos */
    struct rMonthPos {
      QBitArray rDays;
      short rPos;
      bool negative;
    };

    Recurrence( Incidence *parent, int compatVersion = 0 );
    Recurrence( const Recurrence&, Incidence *parent );
    ~Recurrence();

    bool operator==( const Recurrence& ) const;
    bool operator!=( const Recurrence& r ) const  { return !operator==(r); }

    Incidence *parent() { return mParent; }

    /** Return the start of the recurrence */
    QDateTime recurStart() const   { return mRecurStart; }
    /** Set start of recurrence, as a date and time. */
    void setRecurStart(const QDateTime &start);
    /** Set start of recurrence, as a date with no time.
     * Recurrence types which are sub-daily (e.g. rHourly) always have a time;
     * the time is set to 00:00:00 in these cases. */
    void setRecurStart(const QDate &start);
    /** Set whether the recurrence has no time, just a date.
     * Recurrence types which are sub-daily (e.g. rHourly) always have a time
     * and cannot be set to float.
     * N.B. This property is derived by default from the parent incidence,
     * or according to whether a time is specified in setRecurStart(). */
    void setFloats(bool f);
    /**
      Returns whether the recurrence has no time, just a date.
    */
    bool doesFloat() const { return mFloats; }

    /** Set if recurrence is read-only or can be changed. */
    void setRecurReadOnly(bool readOnly) { mRecurReadOnly = readOnly; }
    /** Returns true if the recurrence is read-only, or false if it can be changed. */
    bool recurReadOnly() const  { return mRecurReadOnly; }

    /** Set the calendar file version for backwards compatibility.
     * @param version is the KOrganizer/libkcal version, e.g. 220 for KDE 2.2.0.
     * Specify version = 0 to cancel compatibility mode.
     */
    void setCompatVersion(int version = 0);

    /** Returns the event's recurrence status.  See the enumeration at the top
     * of this file for possible values. */
    ushort doesRecur() const;
    /** Returns true if the date specified is one on which the event will
     * recur. */
    bool recursOnPure(const QDate &qd) const;
    /** Returns true if the date/time specified is one at which the event will
     * recur. Times are rounded down to the nearest minute to determine the result. */
    bool recursAtPure(const QDateTime &) const;
    /** Turns off recurrence for the event. */
    void unsetRecurs();

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
    QDate getNextDate(const QDate& preDate, bool* last = 0) const;
    /** Returns the date and time of the next recurrence, after the specified date/time.
     * If the recurrence has no time, the next date after the specified date is returned.
     * @param preDateTime the date/time after which to find the recurrence.
     * @param last if non-null, *last is set to true if the next recurrence is the
     * last recurrence, else false.
     * Reply = date/time of next recurrence, or invalid date if none.
     */
    QDateTime getNextDateTime(const QDateTime& preDateTime, bool* last = 0) const;
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
    /** Returns the number of recurrences up to and including the date specified. */
    int durationTo(const QDate &) const;
    /** Returns the number of recurrences up to and including the date/time specified. */
    int durationTo(const QDateTime &) const;

    /** Returns the date of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     * @param result if non-null, *result is updated to true if successful,
     * or false if there is no recurrence.
     */
    QDate endDate(bool* result = 0) const;
    /** Returns the date and time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     * @param result if non-null, *result is updated to true if successful,
     * or false if there is no recurrence.
     */
    QDateTime endDateTime(bool* result = 0) const;
    /** Returns a string representing the recurrence end date in the format
     according to the user's locale settings. */
    QString endDateStr(bool shortfmt=true) const;
    /** Sets the date of the last recurrence. The end time is set to the recurrence start time.
     * @param endDate the ending date after which to stop recurring. */
    void setEndDate(const QDate &endDate);
    /** Sets the date and time of the last recurrence.
     * @param endDateTime the ending date/time after which to stop recurring. */
    void setEndDateTime(const QDateTime &endDateTime);

    /** Sets an event to recur minutely.
     * @param _rFreq the frequency to recur, e.g. 2 is every other minute
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setMinutely(int _rFreq, int duration);
    /** Sets an event to recur minutely.
     * @param _rFreq the frequency to recur, e.g. 2 is every other minute
     * @param endDateTime the ending date/time after which to stop recurring
     */
    void setMinutely(int _rFreq, const QDateTime &endDateTime);

    /** Sets an event to recur hourly.
     * @param _rFreq the frequency to recur, e.g. 2 is every other hour
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setHourly(int _rFreq, int duration);
    /** Sets an event to recur hourly.
     * @param _rFreq the frequency to recur, e.g. 2 is every other hour
     * @param endDateTime the ending date/time after which to stop recurring
     */
    void setHourly(int _rFreq, const QDateTime &endDateTime);

    /** Sets an event to recur daily.
     * @param _rFreq the frequency to recur, e.g. 2 is every other day
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setDaily(int _rFreq, int duration);
    /** Sets an event to recur daily.
     * @param _rFreq the frequency to recur, e.g. 2 is every other day
     * @param endDate the ending date after which to stop recurring
     */
    void setDaily(int _rFreq, const QDate &endDate);

    /** Sets an event to recur weekly.
     * @param _rFreq the frequency to recur, e.g. every other week etc.
     * @param _rDays a 7 bit array indicating which days on which to recur (bit 0 = Monday).
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     * @param weekStart the first day of the week (Monday=1 .. Sunday=7, default is Monday).
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, int duration, int weekStart = 1);
    /** Sets an event to recur weekly.
     * @param _rFreq the frequency to recur, e.g. every other week etc.
     * @param _rDays a 7 bit array indicating which days on which to recur (bit 0 = Monday).
     * @param endDate the date on which to stop recurring.
     * @param weekStart the first day of the week (Monday=1 .. Sunday=7, default is Monday).
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, const QDate &endDate, int weekStart = 1);
    /** Returns the first day of the week. Monday=1 .. Sunday=7. */
    int weekStart() const        { return rWeekStart; }
    /** Returns week day mask (bit 0 = Monday). */
    const QBitArray &days() const;

    /** Sets an event to recur monthly.
     * @param type rMonthlyPos or rMonthlyDay
     * @param _rFreq the frequency to recur, e.g. 3 for every third month.
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setMonthly(short type, int _rFreq, int duration);
    /** same as above, but with ending date not number of recurrences */
    void setMonthly(short type, int _rFreq, const QDate &endDate);
    /** Adds a position to the recursMonthlyPos recurrence rule, if it is
     * set.
     * @param _rPos the position in the month for the recurrence, with valid
     * values being 1-5 (5 weeks max in a month).
     * @param _rDays the days for the position to recur on (bit 0 = Monday).
     * Example: _rPos = 2, and bits 0 and 2 are set in _rDays:
     * the rule is to repeat every 2nd Monday and Wednesday in the month.
     */
    void addMonthlyPos(short _rPos, const QBitArray &_rDays);
    /** Adds a position to the recursMonthlyDay list.
     * @param _rDay the date in the month to recur.
     */
    void addMonthlyDay(short _rDay);
    /** Returns list of day positions in months. */
    const QPtrList<rMonthPos> &monthPositions() const;
    /** Returns list of day numbers of a  month. */
    const QPtrList<int> &monthDays() const;

    /** Sets an event to recur yearly.
     * @param type rYearlyMonth, rYearlyPos or rYearlyDay
     * @param freq the frequency to recur, e.g. 3 for every third year.
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setYearly(int type, int freq, int duration);
    /** Sets an event to recur yearly ending at \a endDate. */
    void setYearly(int type, int freq, const QDate &endDate);
    /** Sets an event to recur yearly on a set of months, on the same day of the
     * month as the event start date.
     * The months must be specified by calling addYearlyNum().
     * @param type the way recurrences of February 29th are to be handled in non-leap years.
     * @param freq the frequency to recur, e.g. 3 for every third year.
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setYearlyByDate(Feb29Type type, int freq, int duration);
    /** Sets an event to recur yearly ending at \a endDate. */
    void setYearlyByDate(Feb29Type type, int freq, const QDate &endDate);
    /** Sets an event to recur yearly of a set of months, on the specified day of the month.
     * The months must be specified by calling addYearlyNum().
     * @param day the day of the month for the event
     * @param type the way recurrences of February 29th are to be handled in non-leap years.
     * @param freq the frequency to recur, e.g. 3 for every third year.
     * @param duration the number of times the event is to occur, or -1 to recur indefinitely.
     */
    void setYearlyByDate(int day, Feb29Type type, int freq, int duration);
    /** Sets an event to recur yearly ending at \a endDate. */
    void setYearlyByDate(int day, Feb29Type type, int freq, const QDate &endDate);
    /** Adds position of day or month in year.
     * N.B. for recursYearlyPos, addYearlyMonthPos() must also be called
     * to add positions within the month. */
    void addYearlyNum(short _rNum);
    /** Adds a position to the recursYearlyPos recurrence rule, if it is set.
     * N.B. addYearlyNum() must also be called to add recurrence months.
     * Parameters are the same as for addMonthlyPos().
     */
    void addYearlyMonthPos(short _rPos, const QBitArray &_rDays);
    /** Returns positions of days or months in year. */
    const QPtrList<int> &yearNums() const;
    /** Returns list of day positions in months, for a recursYearlyPos recurrence rule. */
    const QPtrList<rMonthPos> &yearMonthPositions() const;
    /** Returns how yearly recurrences of February 29th are handled. */
    Feb29Type feb29YearlyType() const  { return mFeb29YearlyType; }
    /** Sets the default method for handling yearly recurrences of February 29th. */
    static void setFeb29YearlyTypeDefault(Feb29Type t)  { mFeb29YearlyDefaultType = t; }
    /** Returns the default method for handling yearly recurrences of February 29th. */
    static Feb29Type setFeb29YearlyTypeDefault()  { return mFeb29YearlyDefaultType; }

    /** Returns the number of days in the month which recur.
     * Reply = -1 if the number varies from month to month.
     */
    int  countMonthlyPosDays() const;
    /** Returns the days in a specified month which recur, in numerical order.
     * @param list receives the list of days which recur
     * @param daysInMonth number of days in this month
     * @param startDayOfWeek day of week for the first day in this month.
     */
    void getMonthlyPosDays(QValueList<int>& list, int daysInMonth,
                           int startDayOfWeek) const;
    /** Returns the days in a specified month which recur, in numerical order.
     * @param list receives the list of days which recur
     * @param daysInMonth number of days in this month
     * Reply = true if day numbers varies from month to month.
     */
    bool getMonthlyDayDays(QValueList<int>& list, int daysInMonth) const;
    /** Get the months which recur, in numerical order, for both leap years and non-leap years.
     * N.B. If February 29th recurs on March 1st in non-leap years, February (not March) is
     * included in the non-leap year month list.
     * @param day
     * @param list receives the list of months which recur in non-leap years
     * @param leaplist receives the list of months which recur in leap years
     * Reply = true if February 29th also recurs.
     */
    bool getYearlyMonthMonths(int day, QValueList<int>& list,
                              QValueList<int> &leaplist) const;

    /** Upper date limit for recurrences */
    static const QDate MAX_DATE;

    /**
      Debug output.
    */
    void dump() const;

  protected:
    enum PeriodFunc { END_DATE_AND_COUNT, COUNT_TO_DATE, NEXT_AFTER_DATE };

    class MonthlyData;
    class YearlyMonthData;
    class YearlyPosData;
    class YearlyDayData;

    bool recursSecondly(const QDate &, int secondFreq) const;
    bool recursMinutelyAt(const QDateTime &dt, int minuteFreq) const;
    bool recursDaily(const QDate &) const;
    bool recursWeekly(const QDate &) const;
    bool recursMonthly(const QDate &) const;
    bool recursYearlyByMonth(const QDate &) const;
    bool recursYearlyByPos(const QDate &) const;
    bool recursYearlyByDay(const QDate &) const;

    QDate getNextDateNoTime(const QDate& preDate, bool* last) const;
    QDate getPreviousDateNoTime(const QDate& afterDate, bool* last) const;

    void addMonthlyPos_(short _rPos, const QBitArray &_rDays);
    void setDailySub(short type, int freq, int duration);
    void setYearly_(short type, Feb29Type, int freq, int duration);
    int  recurCalc(PeriodFunc, QDate &enddate) const;
    int  recurCalc(PeriodFunc, QDateTime &endtime) const;
    int  secondlyCalc(PeriodFunc, QDateTime& endtime, int freq) const;
    int  dailyCalc(PeriodFunc, QDate &enddate) const;
    int  weeklyCalc(PeriodFunc, QDate &enddate) const;
    int  weeklyCalcEndDate(QDate& enddate, int daysPerWeek) const;
    int  weeklyCalcToDate(const QDate& enddate, int daysPerWeek) const;
    int  weeklyCalcNextAfter(QDate& enddate, int daysPerWeek) const;
    int  monthlyCalc(PeriodFunc, QDate &enddate) const;
    int  monthlyCalcEndDate(QDate& enddate, MonthlyData&) const;
    int  monthlyCalcToDate(const QDate& enddate, MonthlyData&) const;
    int  monthlyCalcNextAfter(QDate& enddate, MonthlyData&) const;
    int  yearlyMonthCalc(PeriodFunc, QDate &enddate) const;
    int  yearlyMonthCalcEndDate(QDate& enddate, YearlyMonthData&) const;
    int  yearlyMonthCalcToDate(const QDate& enddate, YearlyMonthData&) const;
    int  yearlyMonthCalcNextAfter(QDate& enddate, YearlyMonthData&) const;
    int  yearlyPosCalc(PeriodFunc, QDate &enddate) const;
    int  yearlyPosCalcEndDate(QDate& enddate, YearlyPosData&) const;
    int  yearlyPosCalcToDate(const QDate& enddate, YearlyPosData&) const;
    int  yearlyPosCalcNextAfter(QDate& enddate, YearlyPosData&) const;
    int  yearlyDayCalc(PeriodFunc, QDate &enddate) const;
    int  yearlyDayCalcEndDate(QDate& enddate, YearlyDayData&) const;
    int  yearlyDayCalcToDate(const QDate& enddate, YearlyDayData&) const;
    int  yearlyDayCalcNextAfter(QDate& enddate, YearlyDayData&) const;

  private:

    int   getFirstDayInWeek(int startDay, bool useWeekStart = true) const;
    int   getLastDayInWeek(int endDay, bool useWeekStart = true) const;
    QDate getFirstDateInMonth(const QDate& earliestDate) const;
    QDate getLastDateInMonth(const QDate& latestDate) const;
    QDate getFirstDateInYear(const QDate& earliestDate) const;
    QDate getLastDateInYear(const QDate& latestDate) const;
    int   maxIterations() const;

    // Prohibit copying
    Recurrence(const Recurrence&);
    Recurrence &operator=(const Recurrence&);

    short recurs;                        // should be one of the enums.

    int rWeekStart;                      // day which starts the week, Monday=1 .. Sunday=7
    QBitArray rDays;                     // array of days during week it recurs

    QPtrList<rMonthPos> rMonthPositions; // list of positions during a month
                                         // on which an event recurs

    QPtrList<int> rMonthDays;            // list of days during a month on
                                         // which the event recurs

    QPtrList<int> rYearNums;             // either months/days to recur on for rYearly,
                                         // sorted in numerical order

    int rFreq;                           // frequency of period

    // one of the following must be specified
    int rDuration;                       // num times to recur (inc. first occurrence), -1 = infinite, 0 means end date is set
    QDateTime rEndDateTime;              // date/time at which to end recurrence
    mutable QDateTime mCachedEndDT;      // calculated end date/time for duration > 0
                                         // (calculation can be significant for some recurrences)
    mutable bool mUseCachedEndDT;        // true if mCachedEndDT is up to date

    QDateTime mRecurStart;               // date/time of first recurrence
    bool mFloats;                        // the recurrence has no time, just a date
    bool mRecurReadOnly;
    Feb29Type mFeb29YearlyType;          // how to handle yearly recurrences of February 29th
    static Feb29Type mFeb29YearlyDefaultType;  // default value for mFeb29YearlyType

    // Backwards compatibility for KDE < 3.1.
    int   mCompatVersion;                // calendar file version for backwards compatibility
    short mCompatRecurs;                 // original 'recurs' in old calendar format, or rNone
    int   mCompatDuration;               // original 'rDuration' in old calendar format, or 0

    Incidence *mParent;

    class Private;
    Private *d;
};

}

#endif
