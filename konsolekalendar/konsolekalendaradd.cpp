/***************************************************************************
        konsolekalendaradd.cpp  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2002 by Tuukka Pasanen
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

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include "calendarlocal.h"
#include "calendar.h"
#include "event.h"

#include "konsolekalendaradd.h"

using namespace KCal;
using namespace std;

KonsoleKalendarAdd::KonsoleKalendarAdd( KonsoleKalendarVariables *variables )
{
  m_variables = variables;

}

KonsoleKalendarAdd::~KonsoleKalendarAdd()
{

}

/**
 * Adds event to Calendar
 */

bool KonsoleKalendarAdd::addEvent()
{
  Event *event = new Event();

  event->setDtStart( m_variables->getStartDate() );
  event->setDtEnd( m_variables->getEndDate() );
  event->setDescription( m_variables->getDescription() );
  event->setSummary( m_variables->getSummary() );

  if( m_variables->getCalendar()->addEvent( event ) == true ) {
    kdDebug() << "Event added!" << endl;
  } else {
    kdDebug() << "Can't add event" << endl;
  } // else

  m_variables->getCalendar()->save( m_variables->getCalendarFile() );

  delete event;

  return true;
}

bool KonsoleKalendarAdd::addImportedCalendar()
{

 CalendarLocal importCalendar;

 if( !importCalendar.load( m_variables->getImportFile() ) ){
   kdDebug() << "Can't open file: " << m_variables->getImportFile() << endl;
   return false;
 } else {
   kdDebug() << "Successfully opened file: " << m_variables->getImportFile() << endl;
 } // else

 Event::List eventList( importCalendar.rawEvents() );
 Event *singleEvent;

 if( eventList.count() ) {

   Event::List::ConstIterator it;
   for( it = eventList.begin(); it != eventList.end(); ++it ) {
     singleEvent = *it;

     kdDebug() << "Add event" << endl;
     m_variables->getCalendar()->addEvent( singleEvent );

   } // for

 } // if

 importCalendar.close();
 m_variables->getCalendar()->save( m_variables->getCalendarFile() );

 return true;
}

