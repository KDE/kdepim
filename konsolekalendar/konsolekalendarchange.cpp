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
  bool status = false;

  /* Retrieve event on the basis of the unique string ID */
  Event *event = m_variables->getCalendar()->event( m_variables->getUID() );
  if( event ) {
    if( m_variables->isDryRun() ) {
      cout << i18n("Change Event <Dry Run>:").local8Bit() << endl;
      printSpecs( event );
// TODO: put back after string freeze
//      cout << i18n("To Event <Dry Run>:").local8Bit() << endl;
      printSpecs();
    } else {
       kdDebug() << "konsolekalendarchange.cpp:changeEvent() : " << m_variables->getUID().local8Bit() << endl;

// TODO: put back after string freeze
       if( m_variables->isVerbose() ) {
//	 cout << i18n("Change Event <Verbose>:").local8Bit() << endl;
//	 printSpecs( event );
//	 cout << i18n("To Event <Dry Run>:").local8Bit() << endl;
//	 printSpecs();
       }

       if( m_variables->isStartDateTime() )
	 event->setDtStart( m_variables->getStartDateTime() );
       if( m_variables->isEndDateTime() )
	 event->setDtEnd( m_variables->getEndDateTime() );
       if( m_variables->isSummary() )
	 event->setSummary( m_variables->getSummary() );
       if( m_variables->isDescription() )
	 event->setDescription( m_variables->getDescription() );
       m_variables->getCalendar()->addEvent( event );
       m_variables->getCalendar()->save();
     }
    status = true;
  }

  return( status );
}

void KonsoleKalendarChange::printSpecs( Event *event )
{
  cout << i18n("  UID:   ").local8Bit() << event->uid().local8Bit() << endl;
  cout << i18n("  What:  ").local8Bit() << event->summary().local8Bit() << endl;;
  cout << i18n("  Begin: ").local8Bit() << event->dtStart().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  End:   ").local8Bit() << event->dtEnd().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  Desc:  ").local8Bit() << event->description().local8Bit() << endl;;
}

void KonsoleKalendarChange::printSpecs( )
{
  cout << i18n("  UID:   ").local8Bit() << m_variables->getUID().local8Bit() << endl;
  cout << i18n("  What:  ").local8Bit() << m_variables->getSummary().local8Bit() << endl;;
  cout << i18n("  Begin: ").local8Bit() << m_variables->getStartDateTime().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  End:   ").local8Bit() << m_variables->getEndDateTime().toString(Qt::TextDate).local8Bit() << endl;
  cout << i18n("  Desc:  ").local8Bit() << m_variables->getDescription().local8Bit() << endl;;
}
