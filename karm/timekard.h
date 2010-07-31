/*
 *   This file only:
 *     Copyright (C) 2003  Mark Bucciarelli <mark@hubcapconsutling.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the
 *      Free Software Foundation, Inc.
 *      51 Franklin Street, Fifth Floor
 *      Boston, MA  02110-1301  USA.
 *
 */

#ifndef KARM_TIMEKARD_H
#define KARM_TIMEKARD_H

#undef Color // X11 headers
#undef GrayScale // X11 headers
#include <kprinter.h>
//#include <qdate.h>

#include "karmstorage.h"

class QString;
class QDate;

class TaskView;


/**
 *  Seven consecutive days.
 *
 *  The timecard report prints out one table for each week of data.  The first
 *  day of the week should be read from the KControlPanel.  Currently, it is
 *  hardcoded to Sunday.
 */
class Week
{
  public:
    /** Need an empty constructor to use in a TQValueList. */
    Week();
    Week(TQDate from);
    TQDate start() const;
    TQDate end() const;
    TQValueList<TQDate> days() const;

    /**
     * Returns a list of weeks for the given date range.
     *
     * The first day of the week is picked up from the settings in the
     * KontrolPanel.
     *
     * The list is inclusive; for example, if you pass in a date range of two
     * days, one being a Sunday and the other being a Monday, you will get two
     * weeks back in the list.
     */
    static TQValueList<Week> weeksFromDateRange(const TQDate& from,
        const TQDate& to);

    /**
     *  Return the name of the week.
     *
     *  Uses whatever the user has set up for the long date format in
     *  KControlPanel, prefixed by "Week of".
     */
    TQString name() const;


  private:
    TQDate _start;
};

/**
 *  Routines to output timecard data.
 */
class TimeKard
{
  public:
    TimeKard() {};

    enum WhichTime { TotalTime, SessionTime };

    /**
     * Generates ascii text of task totals, for current task on down.
     *
     * Formatted for pasting into clipboard.
     *
     * @param taskview The view whose tasks need to be formatted.
     *
     * @param justThisTask Only useful when user has picked a root task.  We
     * use this parameter to distinguish between when a user just wants to
     * print the task subtree for a root task and when they want to print
     * all tasks.
     */
    TQString totalsAsText(TaskView* taskview, bool justThisTask, WhichTime which);

    /**
     * Generates ascii text of weekly task history, for current task on down.
     *
     * Formatted for pasting into clipboard.
     */
    TQString historyAsText(TaskView* taskview, const TQDate& from,
        const TQDate& to, bool justThisTask, bool perWeek, bool totalsOnly);

private:
    void printTask(Task *t, TQString &s, int level, WhichTime which);

    void printTaskHistory(const Task *t, const TQMap<TQString, long>& datamap,
                          TQMap<TQString, long>& daytotals,
                          const TQDate& from, const TQDate& to,
                          const int level, TQString& retval, bool totalsOnly);

    TQString sectionHistoryAsText(TaskView* taskview,
                                 const TQDate& sectionFrom, const TQDate& sectionTo,
                                 const TQDate& from, const TQDate& to,
                                 const TQString& name,
                                 bool justThisTask, bool totalsOnly);

  };
#endif // KARM_TIMEKARD_H
