/*******************************************************************************
 *   konsolekalendarchange.cpp                                                 *
 *                                                                             *
 *   KonsoleKalendar is a command line interface to KDE calendars              *
 *   Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>           *
 *   Copyright (C) 2003-2004  Allen Winter <awinterz@users.sourceforge.net>    *
 *                                                                             *
 *   This library is free software; you can redistribute it and/or             *
 *   modify it under the terms of the GNU Lesser General Public                *
 *   License as published by the Free Software Foundation; either              *
 *   version 2.1 of the License, or (at your option) any later version.        *
 *                                                                             *
 *   This library is distributed in the hope that it will be useful,           *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 *   Lesser General Public License for more details.                           *
 *                                                                             *
 *   You should have received a copy of the GNU Lesser General Public          *
 *   License along with this library; if not, write to the Free Software       *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA *
 *                                                                             *
 ******************************************************************************/
#include <stdlib.h>
#include <iostream>

#include <kdebug.h>
#include <klocale.h>

#include "konsolekalendarchange.h"

using namespace KCal;
using namespace std;

KonsoleKalendarChange::KonsoleKalendarChange( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarChange::~KonsoleKalendarChange()
{
}

bool KonsoleKalendarChange::changeEvent()
{
  bool status = false;

  kdDebug() << "konsolekalendarchange.cpp::changeEvent()" << endl;

  /* Retrieve event on the basis of the unique string ID */
  Event *event = m_variables->getCalendar()->event( m_variables->getUID() );
  if ( event ) {
    if ( m_variables->isDryRun() ) {
      cout << i18n("Change Event <Dry Run>:").local8Bit()
           << endl;
      printSpecs( event );

      cout << i18n("To Event <Dry Run>:").local8Bit()
           << endl;
      printSpecs();
    } else {
      kdDebug() << "konsolekalendarchange.cpp:changeEvent() : "
                << m_variables->getUID().local8Bit()
                << endl;

      if ( m_variables->isVerbose() ) {
        cout << i18n("Change Event <Verbose>:").local8Bit()
             << endl;
        printSpecs( event );

        cout << i18n("To Event <Dry Run>:").local8Bit()
             << endl;
        printSpecs();
      }

      if ( m_variables->isStartDateTime() ) {
        event->setDtStart( m_variables->getStartDateTime() );
      }

      if ( m_variables->isEndDateTime() ) {
        event->setDtEnd( m_variables->getEndDateTime() );
      }

      event->setFloats( m_variables->getFloating() );

      if ( m_variables->isSummary() ) {
        event->setSummary( m_variables->getSummary() );
      }

      if ( m_variables->isDescription() ) {
        event->setDescription( m_variables->getDescription() );
      }

      if ( m_variables->isLocation() ) {
        event->setLocation( m_variables->getLocation() );
      }

      if ( m_variables->getCalendar()->addEvent( event ) ) {
        cout << i18n("Success: \"%1\" changed").arg(event->summary().local8Bit()).local8Bit()
             << endl;

        if ( !m_variables->isCalendarResources() ) {
          status =
            m_variables->getCalendar()->save( m_variables->getCalendarFile() );
        } else {
          m_variables->getCalendar()->save();
          status = true;
        }
      } else {
        cout << i18n("Failure: \"%1\" not changed").arg(event->summary().local8Bit()).local8Bit()
             << endl;
      }
    }
  }

  kdDebug() << "konsolekalendarchange.cpp::changeEvent() | Done " << endl;
  return status;
}

void KonsoleKalendarChange::printSpecs( Event *event )
{
  cout << i18n("  UID:   ").local8Bit()
       << event->uid().local8Bit()
       << endl;

  cout << i18n("  What:  ").local8Bit()
       << event->summary().local8Bit()
       << endl;

  cout << i18n("  Begin: ").local8Bit()
       << event->dtStart().toString(Qt::TextDate).local8Bit()
       << endl;

  cout << i18n("  End:   ").local8Bit()
       << event->dtEnd().toString(Qt::TextDate).local8Bit()
       << endl;

  cout << i18n("  Desc:  ").local8Bit()
       << event->description().local8Bit()
       << endl;

  cout << i18n("  Location:  ").local8Bit()
       << event->location().local8Bit()
       << endl;
}

void KonsoleKalendarChange::printSpecs( )
{
  cout << i18n("  UID:   ").local8Bit()
       << m_variables->getUID().local8Bit()
       << endl;

  cout << i18n("  What:  ").local8Bit()
       << m_variables->getSummary().local8Bit()
       << endl;

  cout << i18n("  Begin: ").local8Bit()
       << m_variables->getStartDateTime().toString(Qt::TextDate).local8Bit()
       << endl;

  cout << i18n("  End:   ").local8Bit()
       << m_variables->getEndDateTime().toString(Qt::TextDate).local8Bit()
       << endl;

  cout << i18n("  Desc:  ").local8Bit()
       << m_variables->getDescription().local8Bit()
       << endl;

  cout << i18n("  Location:  ").local8Bit()
       << m_variables->getLocation().local8Bit()
       << endl;
}
