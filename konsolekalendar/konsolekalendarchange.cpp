/***************************************************************************
        konsolekalendarchange.cpp  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2003 by Tuukka Pasanen
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

#include <stdlib.h>
#include <iostream>

#include <kdebug.h>
#include <klocale.h>

#include "konsolekalendarchange.h"

using namespace KCal;
using namespace std;

KonsoleKalendarChange::KonsoleKalendarChange( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
}

KonsoleKalendarChange::~KonsoleKalendarChange()
{
}

bool KonsoleKalendarChange::changeEvent()
{
  // Currently, ALL events at the start datetime are changed

  bool found = false;

  Event::List eventList( ((CalendarLocal *) m_variables->
                         getCalendarResources())->
                         rawEventsForDate( m_variables->getStartDateTime() ) );
  
	
  Event::List::ConstIterator it;

  /*
   * Just to make this shorter
   */

  QTime starttime = m_variables->getStartDateTime().time();
//  QTime endtime = m_variables->getEndDateTime().time();

  //for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *singleEvent = *it;

   /*
    * I don't know if end time check is needed (add if so;)
    * There should be millions of changing stuff in same minute...
    */

   if( starttime.hour() == singleEvent->dtStart().time().hour() &&
        starttime.minute() == singleEvent->dtStart().time().minute() ){

     found = true;

     if( m_variables->isDryRun() ) {
       cout << i18n("Change Event <Dry Run>:").local8Bit() << endl;
       m_variables->printSpecs("Change");
     } else {
       kdDebug() << "konsolekalendarchange.cpp:changeEvent() : " << singleEvent->dtStartStr().local8Bit() << endl;
       singleEvent->setSummary( m_variables->getSummary() );
       singleEvent->setDescription( m_variables->getDescription() );
       m_variables->getCalendar()->addEvent( singleEvent );
       m_variables->getCalendar()->save();
     }// else

   } // if

  //} //for


  return( found );
}

