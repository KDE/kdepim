/***************************************************************************
        konsolekalendardelete.cpp  -  description
           -------------------
    begin                : Sun May 25 2003
    copyright            : (C) 2003 by Tuukka Pasanen
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

#include "konsolekalendardelete.h"
#include <iostream>


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
  Event::List eventList( m_variables->
                         getCalendar()->
                         rawEventsForDate(
                             m_variables->getDate()
                         ) );

  Event::List::ConstIterator it;
  for( it = eventList.begin(); it != eventList.end(); ++it ) {
    Event *singleEvent = *it;
    cout << "---- Delete---" << endl;
    cout <<  singleEvent->dtStartStr().remove(0, (singleEvent->dtStartStr().find(' ', 0, false) + 1) ).local8Bit();
    cout << " - ";
    cout << singleEvent->dtEndStr().remove(0, (singleEvent->dtEndStr().find(' ', 0, false) + 1) ).local8Bit();
    cout << "---- Delete ---" << endl;

    m_variables->getCalendar()->deleteEvent( singleEvent );

  } //for

  return true;
}

