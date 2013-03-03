/*******************************************************************************
 * konsolekalendar.h                                                           *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/

#ifndef KONSOLEKALENDAR_H
#define KONSOLEKALENDAR_H

#include "konsolekalendarvariables.h"

#include <KCalCore/Event>
#include <Akonadi/Calendar/FetchJobCalendar>

#include <QDateTime>

class QTextStream;

/**
 * @file konsolekalendar.h
 * Provides the KonsoleKalendar class definition.
 */

/**
 * @brief
 * The base class of the project.
 * @author Tuukka Pasanen
 */
class KonsoleKalendar
{
  public:
    /**
     * Constructs a KonsoleKalendar object from command line arguments.
     *
     * @param variables is a pointer to a #KonsoleKalendarVariables object
     * containing all the command line arguments.
     */
    explicit KonsoleKalendar( KonsoleKalendarVariables *variables );

    /**
     * Destructor
     */
    ~KonsoleKalendar();

    /**
     * Visualize what we need.
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
    bool isEvent( const QDateTime &startdate, const QDateTime &enddate, const QString &summary );

    /**
     * Creates calendar file (If it doesn't exists)
     */
    bool createCalendar();

    /**
     * Prints the available calendars.
     */
    bool printCalendarList();

  private:
    /**
     * Print event specs for dryrun and verbose options
     */
    void printSpecs();

    /**
     * Creates an akonadi resource of type ical.
     */
    bool createAkonadiResource(const QString &icalFileName);

    /**
     * Prints event list in many formats
     *
     * @param ts is the #QTextStream to be printed
     * @param eventList which event we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEventList( QTextStream *ts, KCalCore::Event::List *eventList, QDate dt );

    /**
     * Prints a single event in many formats
     *
     * @param ts is the #QTextStream to be printed
     * @param event which we should print
     * @param dt is the date to use when printing the event for recurring events
     */
    bool printEvent( QTextStream *ts, const KCalCore::Event::Ptr &event, QDate dt );

    /**
     * Variables that changes stuff in program
     */
    KonsoleKalendarVariables *m_variables;

    /**
     * Calendar file itself
     */
    Akonadi::FetchJobCalendar::Ptr m_calendar;

    /**
     * This is useful if we like to have same day events to same system
     */
    QDate m_saveDate;
};

#endif
