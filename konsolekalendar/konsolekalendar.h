/*******************************************************************************
 * konsolekalendar.h                                                           *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2004  Allen Winter <awinterz@users.sourceforge.net>      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef _KONSOLEKALENDAR_H
#define _KONSOLEKALENDAR_H

#include <qdatetime.h>

#include <kapplication.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/event.h>

#include "konsolekalendar.h"
#include "konsolekalendarvariables.h"

namespace KCal {

/**
 * KonsoleKalendar is the base class of the project
 */
  class KonsoleKalendar
  {
  public:
    KonsoleKalendar(KonsoleKalendarVariables *variables);
    ~KonsoleKalendar();

    /**
     * Visualisez what we need
     */
    bool showInstance();

    /**
     * Imports calendar file
     */
    bool importCalendar();

    /**
     * Add event to calendar
     */
    bool addEvent();

    /**
     * Change event
     */
    bool changeEvent();

    /**
     * Delete event
     */
    bool deleteEvent();

    /**
     * Detect if event already exists
     *
     * @param  startdate Starting date
     * @param  enddate   Ending date
     * @param  summary   Which summary event should have have
     */
    bool isEvent( QDateTime startdate, QDateTime enddate, QString summary );

    /**
     * Creates calendar file (If it doesn't exists)
     */
    bool createCalendar();

  private:

    /**
     * Print event specs for dryrun and verbose options
     */
    void printSpecs();

    /**
     * Prints event list in many formats
     *
     * @param eventlist which event we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEventList( QTextStream *ts, Event::List *eventList, QDate dt );

    /**
     * Prints a single event in many formats
     *
     * @param event which we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEvent( QTextStream *ts, Event *event, QDate dt );

    /**
     * Variables that changes stuff in programm
     */
    KonsoleKalendarVariables *m_variables;

    /**
     * Calendar file itself
     */
    ResourceCalendar *m_Calendar;

    /**
     * This is usefull if we like to have same day events to same system
     */
    QDate m_saveDate;

  };

}
#endif
