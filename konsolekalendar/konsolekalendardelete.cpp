/********************************************************************************
 *   konsolekalendardelete.cpp                                                  *
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
  bool status = false;

  /* Retrieve event on the basis of the unique string ID */
  Event *event = m_variables->getCalendar()->event( m_variables->getUID() );

  if( event ) {
    if( m_variables->isDryRun() ) {
      cout << i18n("Delete Event <Dry Run>:").local8Bit() << endl;
      printSpecs( event );
    } else {
      kdDebug() << "konsolekalendardelete.cpp:deleteEvent() : " << m_variables->getUID().local8Bit() << endl;

      if( m_variables->isVerbose() ) {
// TODO: put back after string freeze
//	cout << i18n("Delete Event <Verbose>:").local8Bit() << endl;
	printSpecs( event );
      }

      m_variables->getCalendar()->deleteEvent( event );
      
      if( !m_variables->isCalendarResources() ){
        m_variables->getCalendar()->save( m_variables->getCalendarFile() );
      } else {
        m_variables->getCalendar()->save();	    
      }
    }
    status = true;
  }
 
  return( status );
}

void KonsoleKalendarDelete::printSpecs( Event *event )
{
  cout << i18n("  UID:   ").local8Bit() << m_variables->getUID().local8Bit() << endl;
  cout << i18n("  What:  ").local8Bit() << event->summary().local8Bit() << endl;;
  cout << i18n("  Begin: ").local8Bit() << event->dtStart().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  End:   ").local8Bit() << event->dtEnd().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  Desc:  ").local8Bit() << event->description().local8Bit() << endl;;
}
