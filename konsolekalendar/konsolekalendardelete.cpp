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
      m_variables->getCalendar()->save();
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
