/*******************************************************************************
 * konsolekalendaradd.cpp                                                      *
 *                                                                             *
 * KonsoleKalendar is a command line interface to KDE calendars                *
 * Copyright (C) 2002-2004  Tuukka Pasanen <illuusio@mailcity.com>             *
 * Copyright (C) 2003-2005  Allen Winter <winter@kde.org>                      *
 *                                                                             *
 * This program is free software; you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by        *
 * the Free Software Foundation; either version 2 of the License, or           *
 * (at your option) any later version.                                         *
 *                                                                             *
 * This program is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                *
 * GNU General Public License for more details.                                *
 *                                                                             *
 * You should have received a copy of the GNU General Public License           *
 * along with this program; if not, write to the Free Software                 *
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. *
 *                                                                             *
 * As a special exception, permission is given to link this program            *
 * with any edition of Qt, and distribute the resulting executable,            *
 * without including the source code for Qt in the source distribution.        *
 *                                                                             *
 ******************************************************************************/
/**
 * @file konsolekalendaradd.cpp
 * Provides the KonsoleKalendarAdd class definition.
 * @author Tuukka Pasanen
 * @author Allen Winter
 */
#include <stdlib.h>
#include <iostream>

#include <qdatetime.h>
#include <qobject.h>

#include <kdebug.h>
#include <klocale.h>

#include <libkcal/calendarlocal.h>
#include <libkcal/calendar.h>
#include <libkcal/event.h>

#include "konsolekalendaradd.h"

using namespace KCal;
using namespace std;

KonsoleKalendarAdd::KonsoleKalendarAdd( KonsoleKalendarVariables *vars )
{
  m_variables = vars;
}

KonsoleKalendarAdd::~KonsoleKalendarAdd()
{
}

/**
 * Adds event to Calendar
 */

bool KonsoleKalendarAdd::addEvent()
{
  bool status = true;

  kdDebug() << "konsolekalendaradd.cpp::addEvent()" << endl;

  if ( m_variables->isDryRun() ) {
    cout << i18n( "Insert Event <Dry Run>:" ).local8Bit()
         << endl;
    printSpecs();
  } else {
    if ( m_variables->isVerbose() ) {
      cout << i18n( "Insert Event <Verbose>:" ).local8Bit()
           << endl;
      printSpecs();
    }

    Event *event = new Event();

    event->setDtStart( m_variables->getStartDateTime() );
    event->setDtEnd( m_variables->getEndDateTime() );
    event->setSummary( m_variables->getSummary() );
    event->setFloats( m_variables->getFloating() );
    event->setDescription( m_variables->getDescription() );
    event->setLocation( m_variables->getLocation() );

    if ( m_variables->getCalendar()->addEvent( event ) ) {
      cout << i18n( "Success: \"%1\" inserted" ).
        arg( m_variables->getSummary() ).local8Bit()
           << endl;

      if ( !m_variables->isCalendarResources() ) {
        status =
          m_variables->getCalendar()->save( m_variables->getCalendarFile() );
      } else {
        m_variables->getCalendar()->save();
      }

    } else {
      cout << i18n( "Failure: \"%1\" not inserted" ).
        arg( m_variables->getSummary() ).local8Bit()
           << endl;
      status = false;
    }
  }

  kdDebug() << "konsolekalendaradd.cpp::addEvent() | Done " << endl;
  return status;
}

bool KonsoleKalendarAdd::addImportedCalendar()
{

  // TODO: reimplement this please..

  /*if ( !m_variables->getCalendar()->load( m_variables->getImportFile() ) ) {
    kdDebug()
      << "konsolekalendaradd.cpp::importCalendar() | "
      << "Can't import file: "
      << m_variables->getImportFile()
      << endl;
    return false;
  } else {
    kdDebug()
      << "konsolekalendaradd.cpp::importCalendar() | "
      << "Successfully imported file: "
      << m_variables->getImportFile()
      << endl;
  }

  if ( !m_variables->isCalendarResources() ) {
    m_variables->getCalendar()->save( m_variables->getCalendarFile() );
  } else {
    m_variables->getCalendar()->save();
  }*/

  return true;
}

void KonsoleKalendarAdd::printSpecs()
{
  cout << i18n( "  What:  %1" ).
    arg( m_variables->getSummary() ).local8Bit()
       << endl;

  cout << i18n( "  Begin: %1" ).
    arg( m_variables->getStartDateTime().toString( Qt::TextDate ) ).local8Bit()
       << endl;

  cout << i18n( "  End:   %1" ).
    arg( m_variables->getEndDateTime().toString( Qt::TextDate ) ).local8Bit()
       << endl;

  if ( m_variables->getFloating() == true ) {
    cout << i18n( "  No Time Associated with Event" ).local8Bit()
         << endl;
  }

  cout << i18n( "  Desc:  %1" ).
    arg( m_variables->getDescription() ).local8Bit()
       << endl;

  cout << i18n( "  Location:  %1" ).
    arg( m_variables->getLocation() ).local8Bit()
       << endl;
}
