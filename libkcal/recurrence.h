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
#ifndef KCAL_RECURRENCE_H
#define KCAL_RECURRENCE_H
// $Id$

#include <qstring.h>
#include <qbitarray.h>
#include <qptrlist.h>

namespace KCal {

class Incidence;

/**
  This class represents a recurrence rule for a calendar incidence.
*/
class Recurrence {
  public:

    /** enumeration for describing how an event recurs, if at all. */
    enum { rNone = 0, rMinutely = 0x001, rHourly = 0x0002, rDaily = 0x0003,
           rWeekly = 0x0004, rMonthlyPos = 0x0005, rMonthlyDay = 0x0006,
           rYearlyMonth = 0x0007, rYearlyDay = 0x0008, rYearlyPos = 0x0009 };

    /** structure for Recurs rMonthlyPos */
    struct rMonthPos {
      QBitArray rDays;
      short rPos;
      bool negative;
    };

    /** Constructs a new recurrence with variables initialized to "sane" values. */
    Recurrence(Incidence *parent, int compatVersion = -1);
    Recurrence(const Recurrence&, Incidence *parent);
    ~Recurrence();

    /** Return the start of the recurrence */
    QDateTime recurStart() const   { return mRecurStart; }
    /** Returns the number of exception dates for the recurrence */
    int recurExDatesCount() const  { return mRecurExDatesCount; }
    /** Set start of recurrence, as a date and time. */
    void setRecurStart(const QDateTime &start);
    /** Set start of recurrence, as a date with no time. */
    void setRecurStart(const QDate &start);
    /** Set whether the recurrence has no time, just a date.
     * N.B. This property is derived by default from the parent incidence,
     * or according to whether a time is specified in setRecurStart(). */
    void setFloats(bool f);
    /** Set if recurrence is read-only or can be changed. */
    void setRecurReadOnly(bool readOnly ) { mRecurReadOnly = readOnly; }
    /** Set number of exception dates. */
    void setRecurExDatesCount(int count ) { if (count >= 0) mRecurExDatesCount = count; }

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

    /** Returns frequency of recurrence, in terms of the recurrence time period type. */
    int frequency() const;
    /** Returns the total number of recurrences, including the initial occurrence. */
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
     */
    QDate endDate() const;
    /** Returns the date and time of the last recurrence.
     * An invalid date is returned if the recurrence has no end.
     */
    QDateTime endDateTime() const;
    /** Returns a string representing the recurrence end date in the format
     according to the users lcoale settings. */
    QString endDateStr(bool shortfmt=true) const;

    /** Sets an event to recur minutely.
     * @var _rFreq the frequency to recur, e.g. 2 is every other minute
     * @var duration the number of times the event is to occur.
     */
    void setMinutely(int _rFreq, int duration);
    /** Sets an event to recur minutely.
     * @var _rFreq the frequency to recur, e.g. 2 is every other minute
     * @var endDateTime the ending date/time after which to stop recurring
     */
    void setMinutely(int _rFreq, const QDateTime &endDateTime);

    /** Sets an event to recur hourly.
     * @var _rFreq the frequency to recur, e.g. 2 is every other hour
     * @var duration the number of times the event is to occur.
     */
    void setHourly(int _rFreq, int duration);
    /** Sets an event to recur hourly.
     * @var _rFreq the frequency to recur, e.g. 2 is every other hour
     * @var endDateTime the ending date/time after which to stop recurring
     */
    void setHourly(int _rFreq, const QDateTime &endDateTime);

    /** Sets an event to recur daily.
     * @var _rFreq the frequency to recur, e.g. 2 is every other day
     * @var duration the number of times the event is to occur.
     */
    void setDaily(int _rFreq, int duration);
    /** Sets an event to recur daily.
     * @var _rFreq the frequency to recur, e.g. 2 is every other day
     * @var endDate the ending date after which to stop recurring
     */
    void setDaily(int _rFreq, const QDate &endDate);

    /** Sets an event to recur weekly.
     * @var _rFreq the frequency to recur, e.g. every other week etc.
     * @var _rDays a 7 bit array indicating which days on which to recur.
     * @var duration the number of times the event is to occur.
     * @var weekStart the first day of the week (default = 1, Monday).
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, int duration, int weekStart = 1);
    /** Sets an event to recur weekly.
     * @var _rFreq the frequency to recur, e.g. every other week etc.
     * @var _rDays a 7 bit array indicating which days on which to recur.
     * @var endDate the date on which to stop recurring.
     * @var weekStart the first day of the week (default = 1, Monday).
     */
    void setWeekly(int _rFreq, const QBitArray &_rDays, const QDate &endDate, int weekStart = 1);
    /** Returns the first day of the week. 1 = Monday, ..., 7 = Sunday. */
    int weekStart() const        { return rWeekStart; }
    /** Returns week day mask (bit 0 = Monday). */
    const QBitArray &days() const;

    /** Sets an event to recur monthly.
     * @var type rMonthlyPos or rMonthlyDay
     * @var _rFreq the frequency to recur, e.g. every third month etc.
     * @var duration the number of times the event is to occur.
     */
    void setMonthly(short type, int _rFreq, int duration);
    /** same as above, but with ending date not number of recurrences */
    void setMonthly(short type, int _rFreq, const QDate &endDate);
    /** Adds a position to the recursMonthlyPos recurrence rule, if it is
     * set.
     * @var _rPos the position in the month for the recurrence, with valid
     * values being 1-5 (5 weeks max in a month).
     * @var _rDays the days for the position to recur on.
     * Example: _rPos = 2, and bits 1 and 3 are set in _rDays.
     * the rule is to repeat every 2nd week on Monday and Wednesday.
     */
    void addMonthlyPos(short _rPos, const QBitArray &_rDays);
    /** Adds a position the the recursMonthlyDay list. */
    void addMonthlyDay(short _rDay);
    /** Returns list of day positions in months. */
    const QPtrList<rMonthPos> &monthPositions() const;
    /** Returns list of day numbers of a  month. */
    const QPtrList<int> &monthDays() const;

    /** Sets an event to recur yearly ending after \a duration recurrences. */
    void setYearly(int type, int freq, int duration);
    /** Sets an event to recur yearly ending at \a endDate. */
    void setYearly(int type, int freq, const QDate &endDate);
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

  protected:
    enum PeriodFunc { END_DATE_AND_COUNT, COUNT_TO_DATE, NEXT_AFTER_DATE };
    struct MonthlyData;     friend struct MonthlyData;
    struct YearlyMonthData; friend struct YearlyMonthData;
    struct YearlyPosData;   friend struct YearlyPosData;
    struct YearlyDayData;   friend struct YearlyDayData;

    bool recursSecondly(const QDate &, int secondFreq) const;
    bool recursMinutelyAt(const QDateTime &dt, int minuteFreq) const;
    bool recursDaily(const QDate &) const;
    bool recursWeekly(const QDate &) const;
    bool recursMonthly(const QDate &) const;
    bool recursYearlyByMonth(const QDate &) const;
    bool recursYearlyByPos(const QDate &) const;
    bool recursYearlyByDay(const QDate &) const;

    void addMonthlyPos_(short _rPos, const QBitArray &_rDays);
    void setDailySub(short type, int freq, int duration);
    void setYearly_(short type, int freq, int duration);
    int  recurCalc(PeriodFunc, QDate &enddate) const;
    int  recurCalc(PeriodFunc, QDateTime &endtime) const;
    int  secondlyCalc(PeriodFunc, QDateTime& endtime, int freq) const;
    int  dailyCalc(PeriodFunc, QDate &enddate) const;
    int  weeklyCalc(PeriodFunc, QDate &enddate) const;
    int  weeklyCalcEndDate(QDate& enddate, int daysPerWeek) const;
    int  weeklyCalcToDate(QDate& enddate, int daysPerWeek) const;
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
    int  countMonthlyPosDays() const;
    void getMonthlyPosDays(QValueList<int>&, int daysInMonth, int startDayOfWeek) const;
    bool getMonthlyDayDays(QValueList<int>&, int daysInMonth, const QValueList<int> *days31 = 0) const;
    bool getYearlyMonthMonths(int day, QValueList<int>&, QValueList<int> &leaplist) const;

    // stuff below here is for recurring events
    // this is a SUBSET of vCalendar and should be expanded...
    short recurs;                        // should be one of the enums.

    int rWeekStart;                      // day which starts the week, 1 = Monday .. 7 = Sunday
    QBitArray rDays;                     // array of days during week it recurs

    QPtrList<rMonthPos> rMonthPositions; // list of positions during a month
                                         // on which an event recurs

    QPtrList<int> rMonthDays;            // list of days during a month on
                                         // which the event recurs

    QPtrList<int> rYearNums;             // either months/days to recur on
                                         // for rYearly

    int rFreq;                           // frequency of period

    // one of the following must be specified
    int rDuration;                       // num times to recur (inc. first occurrence), -1 = infinite
    QDateTime rEndDateTime;              // date/time at which to end recurrence

  private:
    QDateTime mRecurStart;               // date/time of first recurrence
    bool mFloats;                        // the recurrence has no time, just a date
    bool mRecurReadOnly;
    int mRecurExDatesCount;

#ifdef LIBKCAL_BACK_COMPAT
    // Backwards compatibility for KDE < 3.1.
    uint  mCompatVersion;                // calendar file version for backwards compatibility, or ~0
    short mCompatRecurs;                 // original 'recurs' in old calendar format, or rNone
    int   mCompatDuration;               // original 'rDuration' in old calendar format, or 0
#endif

    Incidence *mParent;
};

}

#endif
