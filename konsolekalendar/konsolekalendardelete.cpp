/***************************************************************************
        konsolekalendardelete.cpp  -  description
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

#include "konsolekalendardelete.h"

using namespace KCal;
using namespace std;

KonsoleKalendarDelete::KonsoleKalendarDelete( KonsoleKalendarVariables *variables )
{
  m_variables = variables;
}

KonsoleKalendarDelete::~KonsoleKalendarDelete()
{
}

bool KonsoleKalendarDelete::deleteEvent()
{
  // Currently, ALL events at the start datetime are deleted

  bool found = false;

  Event::List eventList( m_variables->
                         getCalendar()->
                         rawEventsForDate( m_variables->getStartDateTime() ) );
  /*
   * Just to make this shorter
   */

  QTime starttime = m_variables->getStartDateTime().time();
//  QTime endtime = m_variables->getEndDateTime().time();

                     
  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *singleEvent = *it;

    /*
     * I don't know if end time check is needed (add if so;)
     * There should be millions of deleting stuff in same minute...
     */

  if( starttime.hour() == singleEvent->dtStart().time().hour() &&
        starttime.minute() == singleEvent->dtStart().time().minute() ){

    found = true;
    if( m_variables->isDryRun() ) {
      cout << i18n("Delete Event <Dry Run>:").local8Bit() << endl;
      m_variables->printSpecs("Delete");
    } else {
      kdDebug() << "konsolekalendardelete.cpp:deleteEvent() : " << singleEvent->dtStartStr().local8Bit() << endl;
      m_variables->getCalendar()->deleteEvent( singleEvent );
      m_variables->getCalendar()->save( m_variables->getCalendarFile() );
    }// else

  } // if
 
  } //for

  return( found );
}

