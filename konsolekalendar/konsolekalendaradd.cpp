/********************************************************************************
 *   konsolekalendaradd.cpp                                                     *
 *                                                                              *
 *   KonsoleKalendar is console frontend to calendar                            *
 *   Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>            * 
 *   Copyright (C) 2003-2004  Allen Winter                                      *
 *                                                                              *
 *   This library is free software; you can redistribute it and/or              * 
 *   modify it under the terms of the GNU Lesser General Public                 *
 *   License as published by the Free Software Foundation; either               *
 *   version 2.1 of the License, or (at your option) any later version.         *
 *                                                                              *
 *   This library is distributed in the hope that it will be useful,            * 
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU          *
 *   Lesser General Public License for more details.                            *
 *                                                                              *
 *   You should have received a copy of the GNU Lesser General Public           *
 *   License along with this library; if not, write to the Free Software        *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  * 
 *                                                                              *
 ********************************************************************************/

#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/event.h>

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
  kdDebug() << "konsolekalendaradd.cpp::addEvent() | Add stuff " << endl;
	
  if( !m_variables->isDescription() && m_variables->isSummary() ) {
    // If no description is provided, use the summary for the description
    m_variables->setDescription( m_variables->getSummary() );
  }

  if( m_variables->isDryRun() ) {
    cout << i18n("Insert Event <Dry Run>:").local8Bit() << endl;
    printSpecs();
  } else {
    Event *event = new Event();

    if( m_variables->isVerbose() ) {
      cout << i18n("Insert Event <Verbose>:").local8Bit() << endl;
      printSpecs();
    }

    event->setDtStart( m_variables->getStartDateTime() );
    event->setDtEnd( m_variables->getEndDateTime() );
    event->setSummary( m_variables->getSummary() );
    event->setFloats( m_variables->getFloating() );
    event->setDescription( m_variables->getDescription() );
    event->setLocation( m_variables->getDescription() );    

    if( m_variables->getCalendar()->addEvent( event ) ) {
      cout << i18n("Success: \"").local8Bit()
	   << m_variables->getSummary().local8Bit() << i18n("\" inserted").local8Bit() << endl;
    } else {
      cout << i18n("Failure: \"").local8Bit()
	   << m_variables->getSummary().local8Bit() << i18n("\" not inserted").local8Bit() << endl;
    } // else

    
    // Do we need these??
    if( !m_variables->isCalendarResources() ){
      m_variables->getCalendar()->save( m_variables->getCalendarFile() );
    } else {
      m_variables->getCalendar()->save();	    
    }
    delete event;
  }

  kdDebug() << "konsolekalendaradd.cpp::addEvent() | Done " << endl;	
  return true;
}

bool KonsoleKalendarAdd::addImportedCalendar()
{

 CalendarLocal importCalendar;

 if( !importCalendar.load( m_variables->getImportFile() ) ){
   kdDebug() << "konsolekalendaradd.cpp::importCalendar() | Can't open file: " << m_variables->getImportFile() << endl;
   return false;
 } else {
   kdDebug() << "konsolekalendaradd.cpp::importCalendar() | Successfully opened file: " << m_variables->getImportFile() << endl;
 } // else

 Event::List eventList( importCalendar.rawEvents() );
 Event *singleEvent;

 if( eventList.count() ) {

   Event::List::ConstIterator it;
   for( it = eventList.begin(); it != eventList.end(); ++it ) {
     singleEvent = *it;

     m_variables->getCalendar()->addEvent( singleEvent );
     kdDebug() << "konsolekalendaradd.cpp::importCalendar() | Event imported" << endl;

   } // for

 } // if

 importCalendar.close();

 if( !m_variables->isCalendarResources() ){
   m_variables->getCalendar()->save( m_variables->getCalendarFile() );
 } else {
   m_variables->getCalendar()->save();	    
 }

 return true;
}

void KonsoleKalendarAdd::printSpecs()
{
  cout << i18n("  What:  ").local8Bit() << m_variables->getSummary().local8Bit() << endl;
  cout << i18n("  Begin: ").local8Bit() << m_variables->getStartDateTime().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  End:   ").local8Bit() << m_variables->getEndDateTime().toString(Qt::TextDate).local8Bit() << endl;
  if( m_variables->getFloating() == true ) {
    cout << i18n("  No Time Associated with Event").local8Bit() << endl;
  }
  cout << i18n("  Desc:  ").local8Bit() << m_variables->getDescription().local8Bit() << endl;
  cout << i18n("  Location:  ").local8Bit() << m_variables->getLocation().local8Bit() << endl;

}
