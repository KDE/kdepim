#ifndef _KONSOLEKALENDAR_H
#define _KONSOLEKALENDAR_H

/***************************************************************************
        konsolekalendar.h  -  description
           -------------------
    begin                : Sun Jan  6 11:50:14 EET 2002
    copyright            : (C) 2002-2003 by Tuukka Pasanen
    copyright            : (C) 2003 by Allen Winter
    email                : illuusio@mailcity.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qdatetime.h>

#include <kapplication.h>

#include <libkcal/calendarlocal.h>
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
    KonsoleKalendar(KonsoleKalendarVariables &variables);
    ~KonsoleKalendar();

   /**
    * Visualisez what we need
    */
    void showInstance();

   /**
    *  Opens calendar file (now in our machine future from others)
    */
   bool openCalendar();

   /**
    * Closes calendar.
    */
   void closeCalendar();

   /**
    * Imports calendar file
    */

   void importCalendar();

   /**
    * Add event to calendar
    */
   void addEvent();

   /**
    * Change event
    */

   void changeEvent();

   /**
    * Delete event
    */

   void deleteEvent();

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
    * Prints event list in many formats
    *
    * @param eventlist which event we should print
    */

    void printEventList( Event::List *eventList );

   /**
    * Variables that changes stuff in programm
    */
    KonsoleKalendarVariables m_variables;
   /**
    * Calendar file itself
    */
    CalendarLocal *m_Calendar;
};

}
#endif
